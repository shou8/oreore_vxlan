#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>

#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "base.h"
#include "log.h"
#include "netutil.h"



#define CON_NUM		1
#define SUNPATH_LEN	108



const struct in_addr inaddr_any = { INADDR_ANY };



int init_epfd(int max_events) {

	int epfd = epoll_create(max_events);
	if (epfd < 0)
		log_pcrit("epoll_create");

	return epfd;
}



int add_sock(int epfd, int sock) {

	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = sock;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0) {
		log_perr("epoll_ctl");
		return -1;
	}

	return 0;
}



int init_udp_sock(int enable_ipv4, int enable_ipv6, char *port) {

	int sock;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = (enable_ipv6) ? AF_INET6 : AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &res) < 0) {
		log_pcrit("getaddrinfo");
		return -1;
	}

	if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
		log_pcrit("udp.socket");
		return -1;
	}

#ifdef DEBUG
	log_debug("socket      : %d\n", sock);
	log_debug("ai_family   : %d\n", res->ai_family);
	log_debug("ai_socktype : %d\n", res->ai_socktype);
	log_debug("ai_protocol : %d\n", res->ai_protocol);
	log_debug("ai_addrlen  : %d\n", res->ai_addrlen);

	char str[DEFAULT_BUFLEN];
	if (res->ai_family == AF_INET) {
		struct sockaddr_in *addr4 = (struct sockaddr_in *)res->ai_addr;
		log_debug("sin_family  : %d\n", addr4->sin_family);
		log_debug("sin_port    : %d\n", ntohs(addr4->sin_port));
		log_debug("sin_addr    : %s\n", inet_ntop(res->ai_family, &addr4->sin_addr, str, res->ai_addrlen));
	} else {
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)res->ai_addr;
		log_debug("sin6_family : %d\n", addr6->sin6_family);
		log_debug("sin6_port   : %d\n", ntohs(addr6->sin6_port));
		log_debug("sin6_addr   : %s\n", inet_ntop(res->ai_family, &addr6->sin6_addr, str, res->ai_addrlen));
	}
#endif

#ifdef IPV6_V6ONLY
	int on = 1;

	if ( ! enable_ipv4 && enable_ipv6 ) {
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0) {
			log_pcrit("setsockopt");
			log_err("Fail to set IPV6_V6ONLY\n");
			return -1;
		}
	}
#endif

	if (bind(sock, (struct sockaddr *)res->ai_addr, res->ai_addrlen) < 0) {
		log_pcrit("udp.bind");
		close(sock);
		return -1;
	}

	return sock;
}

/*
int init_udp_sock(struct addrinfo *ai) {

	int sock;

	if ((sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
		log_pcrit("udp.socket");
		return -1;
	}

	printf("socket      : %d\n", sock);
	printf("ai_family   : %d\n", ai->ai_family);
	printf("ai_socktype : %d\n", ai->ai_socktype);
	printf("ai_protocol : %d\n", ai->ai_protocol);
	printf("ai_addrlen  : %d\n", ai->ai_addrlen);

	char str[DEFAULT_BUFLEN];
	if (ai->ai_family == AF_INET) {
		struct sockaddr_in *addr4 = (struct sockaddr_in *)ai->ai_addr;
		printf("sin_family  : %d\n", addr4->sin_family);
		printf("sin_port    : %d\n", ntohs(addr4->sin_port));
		printf("sin_addr    : %s\n", inet_ntop(ai->ai_family, &addr4->sin_addr, str, ai->ai_addrlen));
	} else {
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)ai->ai_addr;
		printf("sin6_family : %d\n", addr6->sin6_family);
		printf("sin6_port   : %d\n", ntohs(addr6->sin6_port));
		printf("sin6_addr   : %s\n", inet_ntop(ai->ai_family, &addr6->sin6_addr, str, ai->ai_addrlen));
	}

	if (bind(sock, (struct sockaddr *)ai->ai_addr, ai->ai_addrlen) < 0) {
		log_pcrit("udp.bind");
		close(sock);
		return -1;
	}

	return sock;
}
*/



int init_unix_sock(char *dom, int csflag) {

	int sock;
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX; // AF_LOCAL
	if (dom == NULL)
		strncpy(addr.sun_path, DEFAULT_UNIX_DOMAIN, SUNPATH_LEN);
	else
		strncpy(addr.sun_path, dom, SUNPATH_LEN);

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

int join_mcast4_group(int sock, struct in_addr maddr, char *if_name) {

	struct ip_mreq mreq;
	char maddr_s[16];

	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr = maddr;
	mreq.imr_interface = (if_name != NULL) ? get_addr(if_name) : inaddr_any;

#ifdef DEBUG
	inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
	log_debug("Join mcast_addr4 : %s\n", maddr_s);
#endif
	
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
		log_perr("setsockopt");
		log_err("Fail to set IP_ADD_MEMBERSHIP\n");
		inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
		log_err("mcast_addr: %s\n", maddr_s);
		inet_ntop(AF_INET, &mreq.imr_interface, maddr_s, sizeof(maddr_s));
		log_err("if_addr   : %s\n", maddr_s);
		return -1;
	}

	if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&mreq.imr_interface, sizeof(mreq.imr_interface)) < 0) {
		log_perr("setsockopt");
		log_err("Fail to set IP_MULTICAST_IF\n");
		inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
		log_err("mcast_addr: %s\n", maddr_s);
		inet_ntop(AF_INET, &mreq.imr_interface, maddr_s, sizeof(maddr_s));
		log_err("if_addr   : %s\n", maddr_s);
		return -1;
	}

	return 0;
}



int leave_mcast4_group(int sock, struct in_addr maddr, char *if_name) {

	struct ip_mreq mreq;
	char maddr_s[16];

	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr = maddr;
	mreq.imr_interface = get_addr(if_name);

#ifdef DEBUG
	inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
	log_debug("Leave mcast_addr4: %s\n", maddr_s);
#endif

	if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
		inet_ntop(AF_INET, &mreq.imr_multiaddr, maddr_s, sizeof(maddr_s));
		log_perr("setsockopt");
		log_err("Fail to set IP_DROP_MEMBERSHIP\n");
		log_err("mcast_addr: %s\n", maddr_s);
		return -1;
	}

	return 0;
}




int join_mcast6_group(int sock, struct in6_addr maddr, char *if_name) {

	struct ipv6_mreq mreq6;
	char maddr_s[16];

	memset(&mreq6, 0, sizeof(mreq6));
	mreq6.ipv6mr_multiaddr = maddr;
	mreq6.ipv6mr_interface = (if_name != NULL) ? if_nametoindex(if_name) : 0;

#ifdef DEBUG
	inet_ntop(AF_INET6, &mreq6.ipv6mr_multiaddr, maddr_s, sizeof(maddr_s));
	log_debug("Join mcast_addr6 : %s\n", maddr_s);
#endif

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&mreq6.ipv6mr_interface, sizeof(mreq6.ipv6mr_interface)) < 0) {
		log_perr("setsockopt");
		log_err("Fail to set IPV6_MULTICAST_IF\n");
		inet_ntop(AF_INET6, &mreq6.ipv6mr_multiaddr, maddr_s, sizeof(maddr_s));
		log_err("mcast_addr: %s\n", maddr_s);
		inet_ntop(AF_INET, &mreq6.ipv6mr_interface, maddr_s, sizeof(maddr_s));
		log_err("if_addr   : %s\n", maddr_s);
		return -1;
	}

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&mreq6, sizeof(mreq6)) < 0) {
		log_perr("setsockopt");
		log_err("Fail to set IPV6_ADD_MEMBERSHIP\n");
		log_err("Interface : %u\n", mreq6.ipv6mr_interface);
		return -1;
	}

	return 0;
}



int leave_mcast6_group(int sock, struct in6_addr maddr, char *if_name) {

	struct ipv6_mreq mreq6;
	char maddr_s[16];

	memset(&mreq6, 0, sizeof(mreq6));
	mreq6.ipv6mr_multiaddr = maddr;
	mreq6.ipv6mr_interface = (if_name != NULL) ? if_nametoindex(if_name) : 0;

#ifdef DEBUG
	inet_ntop(AF_INET6, &mreq6.ipv6mr_multiaddr, maddr_s, sizeof(maddr_s));
	log_debug("Join mcast_addr6 : %s\n", maddr_s);
#endif

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char *)&mreq6.ipv6mr_interface, sizeof(mreq6.ipv6mr_interface)) < 0) {
		inet_ntop(AF_INET6, &mreq6.ipv6mr_multiaddr, maddr_s, sizeof(maddr_s));
		log_perr("setsockopt");
		log_err("Fail to set IPV6_DROP_MEMBERSHIP\n");
		log_err("mcast_addr: %s\n", maddr_s);
		return -1;
	}

	return 0;
}



