#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "table.h"
#include "base.h"



#define VNI_BIT		24
#define VNI_BYTE	3



typedef struct _mac2ip4_table_
{
	uint8_t hw_addr[MAC_LEN];
	uint32_t vtep_addr;
	time_t time;
} mac_tbl;



typedef struct _list_
{
	mac_tbl *data;
	struct _list_ *pre;
	struct _list_ *next;
} list;



typedef struct _device_
	int sock;
	char name[256];
	uint8_t hwaddr[MAC_LEN];
} device;



typedef struct _vxlan_instance_
{
	uint8_t vni[VNI_BYTE];
	device dev;
	list **table;
} vxi;



typedef struct _vxlan_list_
{
	vxi *vi;
	struct _vxlan_instance_ *pre;
	struct _vxlan_instance_ *next;
} vlist;



vxi init_vxlan(int vid);



#endif /* VXLAN_H_INCLUDED */

