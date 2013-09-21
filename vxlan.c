#include <stdio.h>

#include "table.h"
#include "vxlan.h"



vxi init_vxlan(int vid)
{
	vxi vi;
	vi.vid = vid;
	vi.sock = 0;
	vi.table = init_table(TABLE_SIZE);

	return vi;
}



/*
int add_data(vxi vi, uint8_t *hw_addr, uint32_t vtep_addr)
{
		return 0;
}
*/
