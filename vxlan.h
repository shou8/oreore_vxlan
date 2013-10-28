#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <netinet/in.h>

#include "base.h"
#include "table.h"



#define VNI_BIT		24
#define VNI_BYTE	3



typedef struct _vxlan_instance_
{
	uint8_t vni[VNI_BYTE];
	uint32_t mcast_addr;
	int usoc;
	device tap;
	list **table;
} vxi;




extern vxi ****vxlan;

vxi ****init_vxlan(void);
void destroy_vxlan(void);
void add_vxi(uint8_t *vni, char *addr);
void del_vxi(uint8_t *vni);
void show_vxi(void);



#endif /* VXLAN_H_INCLUDED */

