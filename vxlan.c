#include <stdio.h>

#include "netutil.h"
#include "vxlan.h"



int cmp_data(TABLE_DATA_TYPE *d1, TABLE_DATA_TYPE *d2)
{
	return cmp_mac(d1->hw_addr, d2->hw_addr);
}
