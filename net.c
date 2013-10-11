#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>

#include "base.h"
#include "log.h"
#include "iftap.h"
#include "netutil.h"
#include "vxlan.h"



#define VXLAN_PORT	4789
#define BUF_SIZE	65536

#define VXLAN_FLAG_MASK 0x08



typedef struct _vxlan_h_
{
	uint8_t flag;
	uint8_t reserve1[3];
	uint8_t vni[3];
	uint8_t reserve2;
} vxlan_h;



int init_raw_sock(char *dev)
{
	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int sock;

	tap_alloc(dev);
	tap_up(dev);

	if ((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		log_pcrit("socket");
		return -1;
	}

	memset(&ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq.ifr_name, dev, IFNAMSIZ-1);
	if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0)
	{
		log_pcrit("ioctl");
		close(sock);
		return -1;
	}

	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_ifindex = ifreq.ifr_ifindex;

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		log_pcrit("bind");
		close(sock);
		return -1;
	}

	return sock;
}



int init_udp_sock(void)
{
	int sock;
	struct sockaddr_in addr;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		log_pcrit("socket");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(VXLAN_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		log_pexit("bind");
		close(sock);
		return -1;
	}

	return sock;
}



int outer_loop(int udp_soc)
{
	int buf_len, len;
	char *bp, *p;
	char buf[BUF_SIZE];
	struct sockaddr_in sender;
	socklen_t addr_len = sizeof(sender);

	while (1)
	{
		if ((buf_len = recvfrom(udp_soc, buf, sizeof(buf)-1, 0,
						(struct sockaddr *)&sender, &addr_len)) < 0)
			log_perr("recvfrom");

		bp = buf;
		vxlan_h *vh = (vxlan_h *)buf;
		bp += sizeof(vxlan_h);
		buf_len -= sizeof(vxlan_h);

		/*
		 * For DEBUG
		 */
//		printf("flag: %02X\n", vh->flag);
//		printf("VNI: %02X%02X%02X\n", vh->vni[0], vh->vni[1], vh->vni[2]);
//		printf("---\n");
		static unsigned long long i;
		printf("%llu\n", i++);

		vxi *v = vxlan[vh->vni[0]][vh->vni[1]][vh->vni[2]];
		if (v == NULL) continue;

		struct ether_header *eh = (struct ether_header *)bp;
		p = bp + sizeof(struct ether_header);
		len = len - sizeof(struct ether_header);

		if (eh->ether_type == ETH_P_ARP) {
			add_data(v->table, eh->ether_shost, sender.sin_addr.s_addr);
//			show_table(v->table);
		}

//printf("blen: %d\n", buf_len);
//		write(v->dev.sock, bp, buf_len);
		send(v->dev.sock, bp, buf_len, MSG_DONTWAIT);

#ifdef DEBUG
//		print_eth_h(eh, stdout);
#endif

//		write(raw_soc, bp, buf_len); 
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



//int packet_analyze(char *buf, int buf_len)
//{
//	struct ether_header *eh;
//}



int inner_loop(int raw_soc)
{
	int rlen, wlen;
	char *rp, *wp;
	char rbuf[BUF_SIZE];
	char wbuf[BUF_SIZE];

	struct sockaddr_in sender;
	socklen_t addr_len = sizeof(sender);
	vxlan_h vh;

	while(1)
	{
		if ((rlen = recvfrom(raw_soc, rbuf, sizeof(rbuf)-1, 0,
						(struct sockaddr *)&sender, &addr_len)) < 0)
			log_perr("recvfrom");

		struct ether_header *eh = (struct ether_header *)rbuf;

		print_eth_h(eh, stdout);

//		vxi *v = vxlan[vh->vni[0]][vh->vni[1]][vh->vni[2]];
//		if (v == NULL) continue;
//
//		p = bp + sizeof(struct ether_header);
//		len = len - sizeof(struct ether_header);
//
//		if (eh->ether_type == ETH_P_ARP) {
//			add_data(v->table, eh->ether_shost, sender.sin_addr.s_addr);
////			show_table(v->table);
//		}
//
////printf("blen: %d\n", buf_len);
////		write(v->dev.sock, bp, buf_len);
//		send(v->dev.sock, bp, buf_len, MSG_DONTWAIT);
//
//#ifdef DEBUG
////		print_eth_h(eh, stdout);
//#endif
//
////		write(raw_soc, bp, buf_len); 

	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}
