#include <stdlib.h>

#include "mpool.h"
#include "netutil.h"
#include "table.h"
#include "log.h"



static mpool_t *pool;
static LIST **table;
static int table_size;



void *init_table(int size) // hash table size
{
	int mem_size = size * sizeof(LIST *);
	table_size = size;

	if ((pool = mp_create(mem_size)) == NULL)
		log_exit("Memory allocation initializing error: mp_create");

	table = (LIST **)mp_alloc(mem_size, pool);

	return table;
}



mac_tbl *find_data(uint8_t *eth_addr)
{
	int key = (int *)eth_addr % table_size;
	LIST *p = *(table + key);

	for ( ; p != NULL; p = p->next)
	{
		mac_tbl *mac_t = p->data;
		uint8_t *eth_p = mac_t->hw_addr;
		if (cmp_mac(eth_p, eth_addr) == 0) return p->data;
	}

	return NULL;
}



int add_data(uint8_t *hw_addr, uint32_t vtep_addr)
{
	mac_tbl *mt = find_data(hw_addr);

	if (mt == NULL)
	{
		mt = (mac_tbl *)mp_alloc(sizeof(mac_tbl), pool);
		memcpy(mt->hw_addr, hw_addr, sizeof(hw_addr));
		mt->vtep_addr = vtep_addr;
		mt->time = time(NULL);

		int key = (int *)hw_addr % table_size;
		LIST *p = *(table + key);
		p->data 
	}
}



#ifdef DEBUG

void show_table(void)
{
	LIST **tp = table;
	LIST *lp;

	int i = 0;
	for( ; i < table_size; i++, tp++)
	{
		printf("%3d: ", i);
		for(lp = *tp; lp != NULL; lp = lp->next)
		{
			printf("%p ->", lp);
		}
		printf("NULL\n");
	}
}

#endif
