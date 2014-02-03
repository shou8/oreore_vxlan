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
#include <sys/un.h>

#include "base.h"
#include "log.h"
#include "iftap.h"
#include "netutil.h"
#include "table.h"
#include "sock.h"
#include "vxlan.h"
#include "net.h"



#define VXLAN_PORT	4789
//#define VXLAN_PORT	60000
#define BUF_SIZE	65536

#define VXLAN_FLAG_MASK		0x08



typedef struct _vxlan_h_ {
	uint8_t flag;
	uint8_t rsv1[3];
	uint8_t vni[VNI_BYTE];
	uint8_t rsv2;
} vxlan_h;



//static int usoc = -1;

#ifdef DEBUG
	static void print_vxl_h(vxlan_h *vh, FILE *fp);
#endif



/*
 * Network Loop Function
 */
int outer_loop(void) {

	char buf[BUF_SIZE], *bp = buf;
	int len;
	int rlen = sizeof(buf) - 1;

	struct sockaddr_in src;
	socklen_t addr_len = sizeof(src);

	vxlan.usoc = init_udp_sock(VXLAN_PORT);
	if (vxlan.usoc < 0) log_cexit("outer_loop.socket: Bad descripter\n");

	if (join_mcast_group(vxlan.usoc, vxlan.mcast_addr, vxlan.if_name) < 0) {
		log_berr(buf, "Cannot initialize socket (%d).\n", vxlan.usoc);
		log_bperr(buf, "socket");
		exit(EXIT_FAILURE);
	}

	while (1) {

		if ((len = recvfrom(vxlan.usoc, buf, rlen, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("recvfrom");

printf("outer raw buf_len: %d\n", len);

		vxlan_h *vh = (vxlan_h *)buf;
		bp = buf + sizeof(vxlan_h);
		len -= sizeof(vxlan_h);

printf("outer aft buf_len: %d\n", len);

#ifdef DEBUG
		if (get_status())
			print_vxl_h(vh, stdout);
#endif

		vxlan_i *v = vxlan.vxi[vh->vni[0]][vh->vni[1]][vh->vni[2]];
		if (v == NULL) continue;

		struct ether_header *eh = (struct ether_header *)bp;
		if (ntohs(eh->ether_type) == ETHERTYPE_ARP) {
			add_data(v->table, eh->ether_shost, ntohl(src.sin_addr.s_addr));
		}

		write(v->tap.sock, bp, len); 

#ifdef DEBUG
		if (get_status())
			print_eth_h(eh, stdout);
#endif
		/*
		 * This function is called by "thread_create".
		 * And this have "while loop", so we have to use "thread_cancel" to stop this.
		 * But, we don't have to worry to use it, because pthread's "canceltype" is "DEFERRED" by default.
		 */
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



int inner_loop(vxlan_i *v) {

	/* Common declaration */
	char buf[BUF_SIZE];
	char *rp = buf + sizeof(vxlan_h);
	int rlen = sizeof(buf) - sizeof(vxlan_h) - 1;
	int len;

	/* For RAW socket declaration (Read) */
	struct sockaddr_in src;
	socklen_t addr_len = sizeof(src);

	struct sockaddr_in dst;
	dst.sin_port = htons(VXLAN_PORT);

	/* For vxlan instance declaration */
	device tap = v->tap;
	int rsoc = tap.sock;
	if (rsoc < 0) {
		log_crit("inner_loop.socket: Bad descripter\n");
		return -1;
	}

	mac_tbl *data;
	vxlan_h *vh = (vxlan_h *)buf;

	while (1) {

		if ((len = recvfrom(rsoc, rp, rlen, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("inner_loop.recvfrom");

		memset(vh, 0, sizeof(vxlan_h));
		vh->flag = VXLAN_FLAG_MASK;
		memcpy(vh->vni, v->vni, VNI_BYTE);
		len += sizeof(vxlan_h);

printf("inner max buf_len: %d\n", rlen);
printf("inner raw buf_len: %d\n", len-sizeof(vxlan_h));
printf("inner aft buf_len: %d\n", len);

#ifdef DEBUG
		if (get_status())
			print_vxl_h(vh, stdout);
#endif

		struct ether_header *eh = (struct ether_header *)rp;

#ifdef DEBUG
		if (get_status())
			print_eth_h(eh, stdout);
#endif

		data = find_data(v->table, eh->ether_dhost);
		if (ntohs(eh->ether_type) == ETHERTYPE_ARP || data == NULL) {
			dst.sin_addr.s_addr = vxlan.mcast_addr;
			if (sendto(vxlan.usoc, buf, len, MSG_DONTWAIT, (struct sockaddr *)&dst, sizeof(struct sockaddr)) < 0)
				log_perr("inner_loop.sendto");
printf("mcast: %d\n", vxlan.usoc);
			continue;
		}

		dst.sin_addr.s_addr = data->vtep_addr;
		if (sendto(vxlan.usoc, buf, len, MSG_DONTWAIT, (struct sockaddr *)&dst, sizeof(struct sockaddr)) < 0)
			log_perr("inner_loop.sendto");
printf("ucast: %d\n", vxlan.usoc);
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



/*
 * Getter
 */
/*
int get_usoc(void) {

	return usoc;
}
*/



void print_vxl_h(vxlan_h *vh, FILE *fp) {

	fprintf(fp, "vxlan_header =====\n");
	fprintf(fp, "flag: %08X\n", vh->flag);
	fprintf(fp, "rsv1: %08X.%08X.%08X\n", vh->rsv1[0], vh->rsv1[1], vh->rsv1[2]);
	fprintf(fp, "vni : %08X.%08X.%08X\n", vh->vni[0], vh->vni[1], vh->vni[2]);
	fprintf(fp, "rsv2: %08X\n", vh->rsv2);
}
