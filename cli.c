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



int main(int argc, char *argv[])
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
		len = sendto(sock, "HELLO", 5, 0, (struct sockaddr *)&addr, sizeof(addr));
		sleep(3);
	}

    return 0;
}

