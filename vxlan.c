#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "table.h"
#include "vxlan.h"



#define TABLE_MIN	1024



static vlist **vt;
static int table_size;

static int cmp_vni(uint8_t *vni1, uint8_t *vni2);



vlist **init_vxlan(unsigned short int size)
{
	table_size = size % USHRT_MAX;
	if (table_size < TABLE_MIN) table_size = TABLE_MIN;
	short int mem_size = table_size * sizeof(vlist *);
	vt = (vlist **)malloc(mem_size);

	return vt;

	vxi vi;
	vi.vid = vid;
	vi.sock = 0;
	vi.table = init_table(TABLE_SIZE);

	return vi;
}



static int cmp_vni(uint8_t *vni1, uint8_t *vni2)
{
	return memcmp(vni1, vni2, VNI_BYTE);
}



static vlist *find_vlist(uint8_t *vni)
{
	short int key = (short int *)vni % table_size;
	vlist *v = *(vxi + (int)key);

	for ( ; v != NULL; v = v->next)
	{
		vxi *vi = v->vi;
		uint8_t *vnip = vi->vni;
		if (cmp_vni(vnip, vni) == 0) return v;
	}

	return NULL;
}



vni *find_vni(uint8_t *vni)
{
	vlist *v = find_vlist(vni);
	if (v != NULL)
		return v->vi;

	return NULL;
}



void add_vni(uint8_t *vni) 
{
}



//int add_data(vxi vi, uint8_t *hw_addr, uint32_t vtep_addr)
//{
//		return 0;
//}
