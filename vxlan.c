#include <stdio.h>

#include "netutil.h"
#include "vxlan.h"



int cmp_data(mac_tbl *d1, mac_tbl *d2)
{
	struct ether_addr *hw_addr1 = d1->hw_addr;
	struct ether_addr *hw_addr2 = d2->hw_addr;

	return cmp_mac(hw_addr1, hwaddr2);
}
