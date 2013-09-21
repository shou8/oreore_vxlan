#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "table.h"
#include "base.h"



typedef struct _mac2ip4_table_
{
	uint8_t hw_addr[MAC_LEN];
	uint32_t vtep_addr;
	time_t time;
} mac_tbl;



typedef struct _list_
{
	mac_tbl *data;
	struct _list_ *next;
} list;



typedef struct _vxlan_instance_
{
	int vid;
	int sock;
	list **table;
} vxi;



vxi init_vxlan(int vid);



#endif /* VXLAN_H_INCLUDED */

