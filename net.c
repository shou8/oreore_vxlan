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

#include "net.h"
#include "log.h"



int init_raw_sock(char *dev);
static int init_udp_sock(void);



int init_raw_sock(char *dev)
{
	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int sock;

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

	if(sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
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



int outr_loop(int sock)
{
	int buf_len;
	char buf[BUF_SIZE];
	struct sockaddr_in sender_info;
	socklen_t addr_len = sizeof(sender_info);

	while (1)
	{
		if ((buf_len = recvfrom(sock, buf, sizeof(buf)-1, 0,
						(struct sockaddr *)&sender_info, &addr_len)) < 0)
			log_perr("recvfrom");
		log_stderr(buf);
	}

	/*
	 * Cannot reach here.
	 */
	return 0;
}
