#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED

#include "vxlan.h"


int init_raw_sock(char *dev);
int init_udp_sock(unsigned short port);
int join_mcast_group(int sock, unsigned short port, char *mcast_addr, char *if_name);
int outer_loop(void);
int inner_loop(vxi *v);
int get_usoc(void);



#endif
