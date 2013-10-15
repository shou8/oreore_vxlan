#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED

#include "vxlan.h"


int init_raw_sock(char *dev);
int init_udp_sock(unsigned short port);
int outer_loop(int udp_soc);
int inner_loop(vxi *v);



#endif
