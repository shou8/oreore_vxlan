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
#include <arpa/inet.h>

#include "base.h"
#include "log.h"
#include "iftap.h"
#include "netutil.h"
#include "table.h"
#include "vxlan.h"



#define VXLAN_PORT	4789
#define BUF_SIZE	65536

#define VXLAN_FLAG_MASK 0x08

#define MCAST_DEFAULT_ADDR 0xef12b500



typedef struct _vxlan_h_
{
	uint8_t flag;
	uint8_t reserve1[3];
	uint8_t vni[3];
	uint8_t reserve2;
} vxlan_h;



static int usoc;



/*
 * Socket Settings
 */



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



int init_udp_sock(unsigned short port)
{
	int sock;
	struct sockaddr_in addr;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		log_pcrit("socket");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		log_pcrit("bind");
		close(sock);
		return -1;
	}

	return sock;
}



/*
 * Multicast Settings
 */

int join_mcast_group(int sock, unsigned short port, char *mcast_addr, char *if_name)
{
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

int outer_loop(void)
{
	int buf_len, len;
	char *bp, *p;
	char buf[BUF_SIZE];
	struct sockaddr_in src;
	socklen_t addr_len = sizeof(src);

	usoc = init_udp_sock(VXLAN_PORT);
	if (usoc < 0) log_cexit("socket: Bad descripter\n");

	while (1)
	{
		if ((buf_len = recvfrom(usoc, buf, sizeof(buf)-1, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("recvfrom");

		bp = buf;
		vxlan_h *vh = (vxlan_h *)buf;
		bp += sizeof(vxlan_h);
		buf_len -= sizeof(vxlan_h);

		vxi *v = vxlan[vh->vni[0]][vh->vni[1]][vh->vni[2]];
		if (v == NULL) continue;

		struct ether_header *eh = (struct ether_header *)bp;
		p = bp + sizeof(struct ether_header);
		len = len - sizeof(struct ether_header);

		if (eh->ether_type == ETH_P_ARP) {
			add_data(v->table, eh->ether_shost, src.sin_addr.s_addr);
//			show_table(v->table);
		}

		send(v->tap.sock, bp, buf_len, MSG_DONTWAIT);

#ifdef DEBUG
//		print_eth_h(eh, stdout);
#endif

		write(v->tap.sock, bp, buf_len); 
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
#include <inttypes.h>



int inner_loop(vxi *v)
{

	/* Common declaration */
	char buf[BUF_SIZE];
	char *rp = buf + sizeof(vxi);
	int rlen = sizeof(buf) - sizeof(vxi) - 1;
	int len;

	/* For RAW socket declaration (Read) */
	struct sockaddr_in src;
	socklen_t addr_len = sizeof(src);

	/* For UDP socket declaration (Write) */
	struct sockaddr_in dst;

	/* For vxlan instance declaration */
	device tap = v->tap;
	int rsoc = tap.sock;
	if (rsoc < 0) log_cexit("socket: Bad descripter\n");

	vxlan_h *vh = (vxlan_h *)buf;
	mac_tbl *data;

	while (1)
	{
		if ((len = recvfrom(rsoc, rp, rlen, 0,
						(struct sockaddr *)&src, &addr_len)) < 0)
			log_perr("recvfrom");

		struct ether_header *eh = (struct ether_header *)rp;

		if ((data = find_data(v->table, eh->ether_dhost)) == NULL)
			continue;

		uint32_t addr = ntohl(data->vtep_addr);
		uint8_t *p = (uint8_t *)&addr;
		printf("vtep: %"PRIu8, *(p++));
		printf(".%"PRIu8, *(p++));
		printf(".%"PRIu8, *(p++));
		printf(".%"PRIu8"\n", *(p++));

	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}



/*
 * Getter
 */
int get_usoc(void)
{
	return usoc;
}
