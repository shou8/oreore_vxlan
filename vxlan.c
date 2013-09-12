#include <stdio.h>

#include "netutil.h"
#include "vxlan.h"



int cmp_data(TABLE_DATA_TYPE *d1, TABLE_DATA_TYPE *d2)
{
	struct ether_addr hw_addr1 = d1->hw_addr;
	struct ether_addr hw_addr2 = d2->hw_addr;

	return cmp_mac(hw_addr1->ether_addr_octet, hw_addr2->ether_addr_octet);
}
