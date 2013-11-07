#ifndef SOCK_H_INCLUDED
#define SOCK_H_INCLUDED



int init_raw_sock(char *dev);
int init_udp_sock(unsigned short port);
int init_unix_sock(char *path);
int join_mcast_group(int sock, unsigned short port, char *mcast_addr, char *if_name);



#endif /* SOCK_H_INCLUDED */
