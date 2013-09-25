#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "netutil.h"
#include "table.h"
#include "log.h"



static list **table;
static int table_size;



void *init_table(int size) // hash table size
{
	int mem_size = size * sizeof(list *);
	table_size = size;
	table = (list **)malloc(mem_size);

	return table;
}



static list *find_list(uint8_t *eth_addr)
{
	int key = *((int *)eth_addr) % table_size;
	list *p = *(table + key);

	for ( ; p != NULL; p = p->next)
	{
		mac_tbl *mac_t = p->data;
		uint8_t *eth_p = mac_t->hw_addr;
		if (cmp_mac(eth_p, eth_addr) == 0) return p;
	}

	return NULL;
}



mac_tbl *find_data(uint8_t *eth)
{
	list *p = find_list(eth);
	if (p != NULL)
		return p->data;

	return NULL;
}



void add_data(uint8_t *hw_addr, uint32_t vtep_addr)
{
	mac_tbl *mt;
	list *mp = find_list(hw_addr);
	int key = *((int *)hw_addr) % table_size;
	list **lr = table + key;
	list *lp = *lr;

	if (mp == NULL)		// Target MAC is not stored
	{
		*lr = (list *)malloc(sizeof(list));

		if (lp != NULL)
		{
			(*lr)->next = lp;
			lp->pre = *lr;
		}

		lp = *lr;
		lp->pre = NULL;
		lp->data = (mac_tbl *)malloc(sizeof(mac_tbl));

		mt = lp->data;
		memcpy(mt->hw_addr, hw_addr, sizeof(hw_addr));
		mt->vtep_addr = vtep_addr;
		mt->time = time(NULL);
	}
	else			// Target MAC exists
	{
		mt = mp->data;
		mt->vtep_addr = vtep_addr;
		memcpy(mt->hw_addr, hw_addr, sizeof(hw_addr));

		if ( lp != mp )
		{
			list *pre = mp->pre;
			pre->next = mp->next;
			if (pre->next != NULL)
				(pre->next)->pre = pre;

			mp->pre = NULL;
			mp->next = lp;
			lp->pre = mp;
			*lr = mp;
		}
	}
}



void del_data(int key)
{
	list **lr = table + key;
	list *p = *lr;

	if (p == NULL) return;
	while( p->next != NULL ) p = p->next;
	list *pre = p->pre;

	if (pre != NULL)
		pre->next = NULL;
	else
		*lr = NULL;

	free(p->data);
	free(p);
}



#ifdef DEBUG

void show_table(void)
{
	list **tp = table;
	list *lp;

	int i = 0;
	for( ; i < table_size; i++, tp++)
	{
		printf("%3d: ", i);
		for(lp = *tp; lp != NULL; lp = lp->next)
		{
			printf("%d => %"PRIu32",  ", ((lp->data)->hw_addr)[0], (lp->data)->vtep_addr);
		}
		printf("NULL\n");
	}
}

#endif
