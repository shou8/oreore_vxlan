#ifndef SOCK_H_INCLUDED
#define SOCK_H_INCLUDED



#define UNIX_SOCK_SERVER 0
#define UNIX_SOCK_CLIENT 1



int init_raw_sock(char *dev);
int init_udp_sock(unsigned short port);
int init_unix_sock(char *path, int csflag);
int join_mcast_group(int sock, unsigned short port, char *mcast_addr, char *if_name);
int leave_mcast_group(int sock, uint32_t mcast_addr, char *if_name);



#endif /* SOCK_H_INCLUDED */
