#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED

#include "vxlan.h"



typedef struct _vxlan_h_ {
	uint8_t flag;
	uint8_t rsv1[3];
	uint8_t vni[VNI_BYTE];
	uint8_t rsv2;
} vxlan_h;



int init_raw_sock(char *dev);
int init_udp_sock(unsigned short port);
int join_mcast_group(int sock, unsigned short port, char *mcast_addr, char *if_name);
int outer_loop(void);
int inner_loop(vxi *v);
int get_usoc(void);
void print_vxl_h(vxlan_h *vh, FILE *fp);



#endif /* NET_H_INCLUDED */
