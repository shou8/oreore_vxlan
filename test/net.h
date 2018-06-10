#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED



#include "vxlan.h"



int outer_loop(void);
int inner_loop(vxlan_i *v);
//int get_usoc(void);



#endif /* NET_H_INCLUDED */
