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



#define VXLAN_PORT	4789



void sendUdp(void);
void make_l2_packet(char *buf);



int main(int argc, char *argv[])
{
	sendUdp();
    return 0;
}



void sendUdp(void)
{
	int len;
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(VXLAN_PORT);
	inet_pton(AF_INET, "192.168.2.11", &addr.sin_addr.s_addr);

	while (1)
	{
		char buf[512];
		make_l2_packet(buf);
		len = sendto(sock, buf, 5, 0, (struct sockaddr *)&addr, sizeof(addr));
		sleep(3);
	}
}



void make_l2_packet(char *buf)
{
	struct ether_header *eh = (struct ether_header *)buf;
	uint8_t *addr = eh->ether_dhost;

	addr[0] = 0;
	addr[1] = 1;
	addr[2] = 2;
	addr[3] = 3;
	addr[4] = 4;
	addr[5] = 5;

	addr = eh->ether_shost;

	addr[0] = 10;
	addr[1] = 11;
	addr[2] = 12;
	addr[3] = 13;
	addr[4] = 14;
	addr[5] = 15;
}
