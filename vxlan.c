#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "tap.h"
#include "sock.h"
#include "net.h"
#include "netutil.h"



/* Written on vxlan.h

typedef struct _vxland {
	int enable_ipv4;
	int enable_ipv6;
	char *port;
	int usoc;
	struct sockaddr_storage m4_addr;
	struct sockaddr_storage m6_addr;
	char *if_name;
	vxlan_i ****vxi;
	char udom[DEFAULT_BUFLEN];
	int lock;
	int timeout;				// Default
	char conf_path[DEFAULT_BUFLEN];
} vxland;
*/

/*
struct sockaddr_in m4_addr_default = {
	AF_INET,					// family (sa_family_t)
	VXLAN_PORT,					// port (in_port_t)
	{ DEFAULT_MCAST_ADDR },		// addr (in_addr)
	{0, 0, 0, 0, 0, 0, 0, 0}	// sin_zero (char[8])
};

struct sockaddr_in6 m6_addr_default = {
	AF_INET6,					// family (sa_family_t)
	VXLAN_PORT,					// port (in_port_t)
	0,							// flowinfo (uint32_t)
	{ { __u6_addr16 { 0xFF15, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001 } } },
	0,							// scope-id: link-local (uint32_t)
};
*/

vxland vxlan = {
	-1,
	-1,
	VXLAN_PORT,

	// IPv4
	0,
	DEFAULT_MCAST_ADDR4,
	{},

	// IPv6
	0,
	DEFAULT_MCAST_ADDR6,
	{},

	NULL,
	NULL,
	DEFAULT_UNIX_DOMAIN,
	0,
	DEFAULT_MAC_TIMEOUT,
	DEFAULT_CONFIG_PATH
};



static void init_vxlan_info(void);
static void init_vxi(void);
static device create_vxlan_if(uint8_t *vni);



int init_vxlan(void) {

	init_vxlan_info();
	init_vxi();
	if ((vxlan.usoc = init_udp_sock(vxlan.enable_ipv4, vxlan.enable_ipv6, vxlan.port)) < 0)
		return -1;

	if (vxlan.enable_ipv4)
		if (join_mcast4_group(vxlan.usoc, ((struct sockaddr_in *)(&vxlan.m4_addr))->sin_addr, vxlan.if_name) < 0)
			return -1;

	if (vxlan.enable_ipv6)
		if (join_mcast6_group(vxlan.usoc, ((struct sockaddr_in6 *)(&vxlan.m6_addr))->sin6_addr, vxlan.if_name) < 0)
			return -1;

	return 0;
}



/*
int init_vxlan(void) {

	init_vxlan_info();
	init_vxi();
	if ((vxlan.repfd = init_epfd(EPOLL_MAX_EVENTS)) < 0)
		return -1;

	struct addrinfo *ai, *res = NULL;
	struct addrinfo hints;

	init_hints(&hints, vxlan.enable_ipv4, vxlan.enable_ipv6);
	if (getaddrinfo(NULL, vxlan.port, &hints, &res) < 0) {
		log_pcrit("getaddrinfo");
		return -1;
	}

	for (ai = res; ai != NULL; ai = ai->ai_next) {
		switch (ai->ai_family) {
			case AF_INET:
				if (vxlan.enable_ipv4) {
					if ((vxlan.usoc4 = init_udp_sock(ai)) < 0)
						return -1;
					if (add_sock(vxlan.repfd, vxlan.usoc4) < 0)
						return -1;
					if (join_mcast4_group(vxlan.usoc4, ((struct sockaddr_in *)(&vxlan.m4_addr))->sin_addr, vxlan.if_name) < 0)
						return -1;
				}
				break;
			case AF_INET6:
				if (vxlan.enable_ipv6) {
					if ((vxlan.usoc6 = init_udp_sock(ai)) < 0)
						return -1;
					if (add_sock(vxlan.repfd, vxlan.usoc6) < 0)
						return -1;
					if (join_mcast6_group(vxlan.usoc6, ((struct sockaddr_in6 *)(&vxlan.m6_addr))->sin6_addr, vxlan.if_name) < 0)
						return -1;
				}
				break;
		}
	}

	freeaddrinfo(res);

	return 0;
}
*/



void init_vxlan_info(void) {

	if ( ! vxlan.enable_ipv4 && ! vxlan.enable_ipv6 ) {
		vxlan.enable_ipv4 = 1;
		vxlan.enable_ipv6 = 1;
		log_info("IPv4 IPv6 multicast address are not specified\n");
		log_info("So, "DAEMON_NAME" prepare dual stack environment\n");
		log_info("and using default IPv4 and IPv6 address\n");
	}

	in_port_t port = (in_port_t)atoi(vxlan.port);
	if (port == 0) {
		log_warn("Invalid port number: %s\n", vxlan.port);
		log_warn("Using default port number: %d\n", VXLAN_PORT);
		strncpy(vxlan.port, VXLAN_PORT, DEFAULT_BUFLEN);
	}

	if (get_sockaddr(&vxlan.m4_addr, vxlan.cm4_addr, vxlan.port) < 0) {
		log_pwarn("getaddrinfo");
		log_warn("Failed to convert 'char *' to 'sockaddr_in'");
		vxlan.enable_ipv4 = 0;
	}

	if (get_sockaddr(&vxlan.m6_addr, vxlan.cm6_addr, vxlan.port) < 0) {
		log_pwarn("getaddrinfo");
		log_warn("Failed to convert 'char *' to 'sockaddr_in6'");
		vxlan.enable_ipv6 = 0;
	}

	return;
}



/*
 * Create 3 Demention Matrix
 */
static void init_vxi(void) {

	vxlan.vxi = (vxlan_i ****)malloc(sizeof(vxlan_i ***) * NUMOF_UINT8);
	if (vxlan.vxi == NULL) log_pcexit("malloc");
	vxlan.vxi[0] = (vxlan_i ***)malloc(sizeof(vxlan_i **) * NUMOF_UINT8 * NUMOF_UINT8);
	if (vxlan.vxi[0] == NULL) log_pcexit("malloc");
	vxlan.vxi[0][0] = (vxlan_i **)malloc(sizeof(vxlan_i *) * NUMOF_UINT8 * NUMOF_UINT8 * NUMOF_UINT8);
	if ( vxlan.vxi[0][0] == NULL ) log_pcexit("malloc");

	int i,j;
	for (i=0; i<NUMOF_UINT8; i++) {
		vxlan.vxi[i] = vxlan.vxi[0] + i * NUMOF_UINT8;
		for (j=0; j<NUMOF_UINT8; j++)
			vxlan.vxi[i][j] = vxlan.vxi[0][0] + i * NUMOF_UINT8 * NUMOF_UINT8 + j * NUMOF_UINT8;
	}

	memset(vxlan.vxi[0][0], 0, sizeof(vxlan_i *) * NUMOF_UINT8 * NUMOF_UINT8 * NUMOF_UINT8);
}



void destroy_vxlan(void) {

	free(vxlan.vxi[0][0]);
	free(vxlan.vxi[0]);
	free(vxlan.vxi);
	vxlan.vxi = NULL;
}



static device create_vxlan_if(uint8_t *vni) {

	device tap;
	uint32_t vni32 = To32ex(vni);

	snprintf(tap.name, IF_NAME_LEN, "vxlan%"PRIu32, vni32);
	//log_debug("VNI: %"PRIu8".%"PRIu8".%"PRIu8"\n", vni[0], vni[1], vni[2], vni32);
	log_info("Tap interface \"%s\" is created (VNI: %"PRIu32").\n", tap.name, vni32);

	tap.sock = tap_alloc(tap.name);
	if (tap.sock < 0) log_cexit("Cannot create tap interface\n");
	tap_up(tap.name);
	get_mac(tap.sock, tap.name, tap.hwaddr);

	return tap;
}



vxlan_i *add_vxi(char *buf, uint8_t *vni, struct sockaddr_storage maddr) {

	vxlan_i *v = (vxlan_i *)malloc(sizeof(vxlan_i));
	if (v == NULL) {
		log_pcrit("malloc");
		return NULL;
	}

	memcpy(v->vni, vni, VNI_BYTE);
	v->table = init_table(DEFAULT_TABLE_SIZE);
	if (v->table == NULL) {
		log_pcrit("malloc");
		free(v);
		return NULL;
	}

	v->tap = create_vxlan_if(vni);
	v->timeout = vxlan.timeout;
	v->maddr = maddr;

	vxlan.vxi[vni[0]][vni[1]][vni[2]] = v;

	return v;
}



void del_vxi(char *buf, uint8_t *vni) {

/*
	if (vxlan.vxi[vni[0]][vni[1]][vni[2]]->mcast_addr.s_addr != vxlan.mcast_addr.s_addr) {

		int i, j, k;
		for (i=0; i<NUMOF_UINT8; i++) {
			for (j=0; j<NUMOF_UINT8; j++) {
				for (k=0; k<NUMOF_UINT8; k++) {
					if (vxlan.vxi[i][j][k] == NULL) continue;
					if (vxlan.vxi[i][j][k]->mcast_addr.s_addr == vxlan.vxi[vni[0]][vni[1]][vni[2]]->mcast_addr.s_addr)
						break;
				}
			}
		}

		if ( i != NUMOF_UINT8 || j != NUMOF_UINT8 || k != NUMOF_UINT8)
			leave_mcast_group(vxlan.usoc, vxlan.vxi[vni[0]][vni[1]][vni[2]]->mcast_addr, vxlan.if_name);
	}
*/

	close(vxlan.vxi[vni[0]][vni[1]][vni[2]]->tap.sock);
	free(vxlan.vxi[vni[0]][vni[1]][vni[2]]);
	vxlan.vxi[vni[0]][vni[1]][vni[2]] = NULL;
}



