#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
//#include <netinet/in.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>

#include "base.h"
#include "log.h"
#include "iftap.h"
#include "netutil.h"
#include "table.h"
#include "vxlan.h"
#include "net.h"



#define VXLAN_PORT	4789
#define BUF_SIZE	65536

#define VXLAN_FLAG_MASK		0x08

#define MCAST_DEFAULT_ADDR	0xef12b500



static int usoc = -1;



/*
 * Socket Settings
 */



int init_raw_sock(char *dev) {

	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int sock;

	tap_alloc(dev);
	tap_up(dev);

	if ((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
		log_pcrit("socket");
		return -1;
	}

	memset(&ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq.ifr_name, dev, IFNAMSIZ-1);
	if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0) {
		log_pcrit("ioctl");
		close(sock);
		return -1;
	}

	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_ifindex = ifreq.ifr_ifindex;

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		log_pcrit("bind");
		close(sock);
		return -1;
	}

	return sock;
}



int init_udp_sock(unsigned short port) {

	int sock;
	struct sockaddr_in addr;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		log_pcrit("socket");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		log_pcrit("bind");
		close(sock);
		return -1;
	}

	return sock;
}



/*
 * Multicast Settings
 */

int join_mcast_group(int sock, unsigned short port, char *mcast_addr, char *if_name) {

	struct ip_mreq mreq;
	static uint32_t maddr = MCAST_DEFAULT_ADDR;

	if (sock <= 0) sock = init_udp_sock(port);

	memset(&mreq, 0, sizeof(mreq));
	if (mcast_addr == NULL) {
		struct in_addr tmp_addr;
		char addr_str[32];
	
		tmp_addr.s_addr = htonl(maddr);
		if( inet_ntop(AF_INET, &tmp_addr.s_addr, addr_str, sizeof(addr_str)) == NULL ) {
			log_perr("Invalid address");
			return -1;
		}

		mcast_addr = addr_str;
		log_warn("Multicast address is automatically generated: %s\n", mcast_addr);
		maddr++;
	}

	if (inet_aton(mcast_addr, &mreq.imr_multiaddr) == 0) {
		log_err("Invalid multicast address: %s", mcast_addr);
		return -1;
	}

	mreq.imr_interface.s_addr = htonl((if_name == NULL) ? INADDR_ANY : get_addr(if_name));
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != 0) {
		log_perr("setsockopt");
		return -1;
	}

	log_info("Multicast address is set: %s\n", mcast_addr);

	return sock;
}



/*
 * Network Loop Function
 */

int outer_loop(void) {

	int buf_len;
	//int len;
	//char *p;
	char buf[BUF_SIZE], *bp;
	struct sockaddr_in src;
	socklen_t addr_len = sizeof(src);

	bp = buf;
	usoc = init_udp_sock(VXLAN_PORT);
	if (usoc < 0) log_cexit("socket: Bad descripter\n");

	while (1) {

		if ((buf_len = recvfrom(usoc, buf, sizeof(buf)-1, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("recvfrom");

		vxlan_h *vh = (vxlan_h *)buf;
		bp = buf + sizeof(vxlan_h);
		buf_len -= sizeof(vxlan_h);
#ifdef DEBUG
		print_vxl_h(vh, stdout);
#endif

		vxi *v = vxlan[vh->vni[0]][vh->vni[1]][vh->vni[2]];
		if (v == NULL) continue;

		struct ether_header *eh = (struct ether_header *)bp;

		/* Store VTEP information */
		if (eh->ether_type == ETH_P_ARP) {
			add_data(v->table, eh->ether_shost, src.sin_addr.s_addr);
		}

		send(v->tap.sock, bp, buf_len, MSG_DONTWAIT);

#ifdef DEBUG
		print_eth_h(eh, stdout);
#endif
		/*
		 * This function is called by "thread_create".
		 * And this have "while loop", so we have to use "thread_cancel" to stop this.
		 * But, we don't have to worry to use it, because pthread's "canceltype" is "DEFERRED" by default.
		 */

//		write(v->tap.sock, bp, buf_len); 
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



int inner_loop(vxi *v) {

	/* Common declaration */
	char buf[BUF_SIZE];
	char *rp = buf + sizeof(vxlan_h);
	int rlen = sizeof(buf) - sizeof(vxlan_h) - 1;
	int len;

	/* For RAW socket declaration (Read) */
	struct sockaddr_in src;
	socklen_t addr_len = sizeof(src);

	/* For UDP socket declaration (Write) */
	if (usoc < 0) usoc = init_udp_sock(VXLAN_PORT);
	if (usoc < 0) log_cexit("socket: Bad descripter\n");
	struct sockaddr_in dst;
	dst.sin_port = VXLAN_PORT;

	/* For vxlan instance declaration */
	device tap = v->tap;
	int rsoc = tap.sock;
	if (rsoc < 0) log_cexit("socket: Bad descripter\n");

	mac_tbl *data;

	/* Initialize vxlan header */
	vxlan_h *vh = (vxlan_h *)buf;

	while (1) {

		if ((len = recvfrom(rsoc, rp, rlen, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("recvfrom");

		memset(vh, 0, sizeof(vxlan_h));
		vh->flag = VXLAN_FLAG_MASK;
		memcpy(vh->vni, v->vni, VNI_BYTE);

#ifdef DEBUG
		print_vxl_h(vh, stdout);
#endif

		struct ether_header *eh = (struct ether_header *)rp;
		if (eh->ether_type == ETH_P_ARP) {
			dst.sin_addr.s_addr = v->mcast_addr;
			if (sendto(usoc, buf, sizeof(vxlan_h)+len, MSG_DONTWAIT, (struct sockaddr *)&dst, sizeof(struct sockaddr)) < 0)
				log_perr("sendto");
			continue;
		}

		if ((data = find_data(v->table, eh->ether_dhost)) == NULL)
			continue;

		dst.sin_addr.s_addr = htonl(data->vtep_addr);
		if (sendto(usoc, buf, sizeof(vxlan_h)+len, MSG_DONTWAIT, (struct sockaddr *)&dst, sizeof(struct sockaddr)) < 0)
			log_perr("sendto");
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



/*
 * Getter
 */
int get_usoc(void) {

	return usoc;
}



void print_vxl_h(vxlan_h *vh, FILE *fp) {

	fprintf(fp, "vxlan_header =====\n");
	fprintf(fp, "flag: %08X\n", vh->flag);
	fprintf(fp, "rsv1: %08X.%08X.%08X\n", vh->rsv1[0], vh->rsv1[1], vh->rsv1[2]);
	fprintf(fp, "vni : %08X.%08X.%08X\n", vh->vni[0], vh->vni[1], vh->vni[2]);
	fprintf(fp, "rsv2: %08X\n", vh->rsv2);
}
