#include <stdlib.h>

#include "mpool.h"
#include "netutil.h"
#include "table.h"
#include "log.h"



static mpool_t *pool;
static TABLE *table;
static int table_size;



void *init_table(int size) // hash table size
{
	int mem_size = size * sizeof(TABLE);
	table_size = size;

	if ((pool = mp_create(mem_size)) == NULL)
		log_exit("Memory allocation initializing error: mp_create");

	table = (TABLE *)mp_alloc(mem_size, pool);

	return table;
}



static int cmp_data(struct ether_addr *eth1, struct ether_addr *eth2)
{
	uint8_t *hw_addr1 = eth1->ether_addr_octet;
	uint8_t *hw_addr2 = eth2->ether_addr_octet;

	return cmp_mac(hw_addr1, hw_addr2);
}



TABLE_DATA_TYPE *find_data(int tbl_key, struct ether_addr *data)
{
	TABLE *p;
	int key = tbl_key % table_size;

	for (p = table + key; p != NULL; p = p->next)
	{
		mac_tbl *mac_tbl_p = &(p->data);
		struct ether_addr *ethp = &(mac_tbl_p->hw_addr);
		if (cmp_data(ethp, data) == 0) return p;
	}

	return NULL;
}
