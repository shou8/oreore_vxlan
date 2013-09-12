#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>



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



int cmp_data(mac_tbl *d1, mac_tbl *d2);



#endif /* VXLAN_H_INCLUDED */

