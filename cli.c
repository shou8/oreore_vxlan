#include <stdio.h>
#include <stdlib.h>
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

typedef struct _vxlan_h_
{
	char flag;
	char reserve1[3];
	char vni[3];
	char reserve2;
} vxlan_h;


void sendUdp(void);
void make_l2_packet(char *buf);
int make_vxlan_header(char *buf);



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
		char buf[128];
		memset(buf, 0, 128);
		int len = make_vxlan_header(buf);
		make_l2_packet(buf + len);
		len = sendto(sock, buf, 128, 0, (struct sockaddr *)&addr, sizeof(addr));
		usleep(100000);
	}
}



// VNI: 100
int make_vxlan_header(char *buf)
{
	static char c = 0;
	vxlan_h *v = (vxlan_h *)buf;
	v->flag = 0x08;
	v->vni[0] = 1;
	v->vni[1] = 0;
	v->vni[2] = 0;

	return sizeof(vxlan_h);
}

void make_l2_packet(char *buf)
{
	struct ether_header *eh = (struct ether_header *)buf;
	uint8_t *addr = eh->ether_dhost;
	eh->ether_type = ETH_P_ARP;

	addr[0] = 0x0;
	addr[1] = 0x1;
	addr[2] = 0x2;
	addr[3] = 0x3;
	addr[4] = 0x4;
	addr[5] = 0x5;

	addr = eh->ether_shost;

	addr[0] = rand() % 256;
	addr[1] = rand() % 256;
	addr[2] = rand() % 256;
	addr[3] = rand() % 256;
	addr[4] = rand() % 256;
	addr[5] = rand() % 256;

	buf[sizeof(struct ether_header) + 1] = '\0';
}
