#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

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



#define CON_NUM		1



int init_raw_sock(char *dev) {

	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int sock;

	memset(&sa, 0, sizeof(sa));
	if ((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
		log_pcrit("raw.socket");
		return -1;
	}

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, dev, IFNAMSIZ-1);
	if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0) {
		log_pcrit("raw.ioctl");
		close(sock);
		return -1;
	}

	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_ifindex = ifreq.ifr_ifindex;

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		log_pcrit("raw.bind");
		close(sock);
		return -1;
	}

	return sock;
}



int init_udp_sock(unsigned short port) {

	int sock;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		log_pcrit("udp.socket");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		log_pcrit("udp.bind");
		close(sock);
		return -1;
	}

printf("outer socket: %d\n", sock);
	return sock;
}



int init_unix_sock(char *dom, int csflag) {

	int sock;
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX; // AF_LOCAL
	if (dom == NULL)
		strncpy(addr.sun_path, DEFAULT_UNIX_DOMAIN, strlen(DEFAULT_UNIX_DOMAIN));
	else
		strncpy(addr.sun_path, dom, strlen(dom));

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		log_pcrit("unix.socket");
		return -1;
	}

	if (csflag == 0) {

		unlink(addr.sun_path);

		if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
			log_pcrit("unix.bind");
			close(sock);
			return -1;
		}

		if (listen(sock, CON_NUM) < 0) {
			log_pcrit("unix.listen");
			close(sock);
			return -1;
		}

	} else {

		if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
			log_pcrit("unix.connect");
			close(sock);
			return -1;
		}
	}

	return sock;
}



/*
 * Multicast Settings
 */

int join_mcast_group(int sock, uint32_t maddr, char *if_name) {

	struct ip_mreq mreq;
	char maddr_s[16];

	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = maddr;
	mreq.imr_interface.s_addr = get_addr(if_name);

	if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&mreq, sizeof(mreq)) != 0) {
		log_perr("setsockopt");
		log_err("Fail to set IP_MULTICAST_IF\n");
		log_err("socket    : %d\n", sock);
		inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
		log_err("mcast_addr: %s\n", maddr_s);
		inet_ntop(AF_INET, &mreq.imr_interface, maddr_s, sizeof(maddr_s));
		log_err("if_addr   : %s\n", maddr_s);
		return -1;
	}
	
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != 0) {
		log_perr("setsockopt");
		log_err("Fail to set IP_ADD_MEMBERSHIP\n");
		log_err("socket    : %d\n", sock);
		inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
		log_err("mcast_addr: %s\n", maddr_s);
		inet_ntop(AF_INET, &mreq.imr_interface, maddr_s, sizeof(maddr_s));
		log_err("if_addr   : %s\n", maddr_s);
		return -1;
	}

	return 0;
}



int leave_mcast_group(int sock, uint32_t maddr, char *if_name) {

	struct ip_mreq mreq;
	char maddr_s[16];

	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = maddr;
	mreq.imr_interface.s_addr = get_addr(if_name);

	if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != 0) {
		inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
		log_perr("setsockopt");
		log_err("Fail to set IP_ADD_MEMBERSHIP\n");
		log_err("socket    : %d\n", sock);
		log_err("mcast_addr: %s\n", maddr_s);
		return -1;
	}

	return 0;
}
