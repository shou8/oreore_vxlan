#ifndef SOCK_H_INCLUDED
#define SOCK_H_INCLUDED



#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>



#define UNIX_SOCK_SERVER 0
#define UNIX_SOCK_CLIENT 1



int init_udp_sock(sa_family_t family, char *port);
int init_unix_sock(char *path, int csflag);
int join_mcast4_group(int sock, struct in_addr maddr, char *if_name);
int leave_mcast4_group(int sock, struct in_addr maddr, char *if_name);
int join_mcast6_group(int sock, struct in6_addr maddr, char *if_name);
int leave_mcast6_group(int sock, struct in6_addr maddr, char *if_name);


#endif /* SOCK_H_INCLUDED */
