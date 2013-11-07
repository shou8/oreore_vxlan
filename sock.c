#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netpacket/packet.h>

#include <sys/un.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
//#include <sys/types.h>
#include <sys/socket.h>

#include "base.h"
#include "log.h"
#include "netutil.h"



#define MCAST_DEFAULT_ADDR	0xef12b500



int init_raw_sock(char *dev) {

	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int sock;

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



int init_unix_sock(char *path) {

	int sock;
	struct sockaddr_un addr;

	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		log_pcrit("socket");
		return -1;
	}

	addr.sun_family = AF_UNIX; // AF_LOCAL
	strncpy(addr.sun_path, path, sizeof(path) - 1);
	unlink(addr.sun_path);

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


