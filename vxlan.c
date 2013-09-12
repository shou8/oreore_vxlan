#include <stdio.h>

#include "netutil.h"
#include "vxlan.h"



int cmp_data(mac_tbl *d1, mac_tbl *d2)
{
	return cmp_mac(d1->hw_addr, d2->hw_addr);
}
