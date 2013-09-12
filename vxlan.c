#include <stdio.h>

#include "netutil.h"
#include "vxlan.h"



int cmp_data(void *data1, void *data2)
{
	mac_tbl *d1 = (mac_tbl *)data1;
	mac_tbl *d2 = (mac_tbl *)data2;

	struct ether_addr hw_addr1 = d1->hw_addr;
	struct ether_addr hw_addr2 = d2->hw_addr;

	return ();
}
