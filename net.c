#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <linux/if_vlan.h>
#include <sys/epoll.h>

#include "base.h"
#include "log.h"
#include "tap.h"
#include "netutil.h"
#include "table.h"
#include "sock.h"
#include "vxlan.h"
#include "net.h"



#define BUF_SIZE			65536
#define VXLAN_FLAG_MASK		0x08



typedef struct _vxlan_h_ {
	uint8_t flag;
	uint8_t rsv1[3];
	uint8_t vni[VNI_BYTE];
	uint8_t rsv2;
} vxlan_h;



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

	struct sockaddr_storage src;
	socklen_t addr_len = sizeof(src);

	while (1) {

		if ((len = recvfrom(vxlan.usoc, buf, rlen, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("recvfrom");

		vxlan_h *vh = (vxlan_h *)buf;
		bp = buf + sizeof(vxlan_h);
		len -= sizeof(vxlan_h);

#ifdef DEBUG
		if (get_status())
			print_vxl_h(vh, stdout);
#endif

		vxlan_i *v = vxlan.vxi[vh->vni[0]][vh->vni[1]][vh->vni[2]];
		if (v == NULL) continue;

		struct ether_header *eh = (struct ether_header *)bp;
		if ( add_data(v->table, eh->ether_shost, &src) < 0 )
			log_pcexit("malloc");

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

	mac_tbl *data;
	vxlan_h *vh = (vxlan_h *)buf;

	while (1) {

		if ((len = read(v->tap.sock, rp, rlen)) < 0)
			log_perr("inner_loop.recvfrom");

		memset(vh, 0, sizeof(vxlan_h));
		vh->flag = VXLAN_FLAG_MASK;
		memcpy(vh->vni, v->vni, VNI_BYTE);
		len += sizeof(vxlan_h);

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
		if (data == NULL) {
			if (sendto(vxlan.usoc, buf, len, MSG_DONTWAIT, (struct sockaddr *)&v->maddr, sizeof(v->maddr)) < 0)
				log_perr("inner_loop.sendto");
			continue;
		}

		if (sendto(vxlan.usoc, buf, len, MSG_DONTWAIT, (struct sockaddr *)&data->vtep_addr, sizeof(data->vtep_addr)) < 0)
			log_perr("inner_loop.sendto");
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



void print_vxl_h(vxlan_h *vh, FILE *fp) {

	fprintf(fp, "vxlan_header =====\n");
	fprintf(fp, "flag: %02X\n", vh->flag);
	fprintf(fp, "rsv1: %02X.%02X.%02X\n", vh->rsv1[0], vh->rsv1[1], vh->rsv1[2]);
	fprintf(fp, "vni : %02X.%02X.%02X\n", vh->vni[0], vh->vni[1], vh->vni[2]);
	fprintf(fp, "rsv2: %02X\n", vh->rsv2);
}



