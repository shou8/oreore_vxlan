#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED

typedef struct _mac2ip4_table_
{
	struct 
} mac_tbl;

typedef mac_tbl HASH_DATA_TYPE;



typedef struct _vxlan_instance_
{
	int vid;
	int sock;
	mac_tbl *mac_tbl;
} vxi;



#endif /* VXLAN_H_INCLUDED */

