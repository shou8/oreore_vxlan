#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <net/in.h>
#include <net/ip.h>
#include <net/ethernet.h>



typedef struct _mac2ip4_table_
{
	struct ether_addr hw_addr;
	in_addr_t vtep_addr;
	time_t time;
} mac_tbl;

typedef mac_tbl TABLE_DATA_TYPE;



typedef struct _vxlan_instance_
{
	int vid;
	int sock;
	mac_tbl *mac_tbl;
} vxi;



#endif /* VXLAN_H_INCLUDED */

