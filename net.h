#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED



#define MCAST_DEFAULT_ADDR	0xef12b500



#include "vxlan.h"



int outer_loop(void);
int inner_loop(vxi *v);
int get_usoc(void);



#endif /* NET_H_INCLUDED */
