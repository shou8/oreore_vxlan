#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "tap.h"
#include "sock.h"
#include "net.h"
#include "netutil.h"



#define VXLAN_PORT	4789



/* Written on vxlan.h

typedef struct _vxland {
	int port;
	int usoc;
	struct in_addr mcast_addr;
	char if_name[IF_NAME_LEN];
	vxlan_i ****vxi;
	char udom[DEFAULT_BUFLEN];
	int lock;
	int timeout;
	char conf_path[DEFAULT_BUFLEN];
} vxland;
*/
vxland vxlan = {
	VXLAN_PORT,
	-1,
	{ DEFAULT_MCAST_ADDR },
	"eth0",
	NULL,
	DEFAULT_UNIX_DOMAIN,
	0,
	DEFAULT_MAC_TIMEOUT,
	DEFAULT_CONFIG_PATH
};



static device create_vxlan_if(uint8_t *vni);




/*
 * Create 3 Demention Matrix
 */
void init_vxlan(void) {

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



vxlan_i *add_vxi(char *buf, uint8_t *vni, char *maddr) {

	vxlan_i *v = (vxlan_i *)malloc(sizeof(vxlan_i));
	if (v == NULL) log_pcexit("malloc");
	memcpy(v->vni, vni, VNI_BYTE);
	v->table = init_table(DEFAULT_TABLE_SIZE);
	if (v->table == NULL) log_pcexit("malloc");
	v->tap = create_vxlan_if(vni);
	v->timeout = vxlan.timeout;
	if (maddr != NULL) {
		inet_aton(maddr, &v->mcast_addr);
		join_mcast_group(vxlan.usoc, v->mcast_addr, vxlan.if_name);
	} else {
		v->mcast_addr = vxlan.mcast_addr;
	}

	vxlan.vxi[vni[0]][vni[1]][vni[2]] = v;

	return v;
}



void del_vxi(char *buf, uint8_t *vni) {

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

	close(vxlan.vxi[vni[0]][vni[1]][vni[2]]->tap.sock);
	free(vxlan.vxi[vni[0]][vni[1]][vni[2]]);
	vxlan.vxi[vni[0]][vni[1]][vni[2]] = NULL;
}



