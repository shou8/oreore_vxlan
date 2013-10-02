#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include "netutil.h"
#include "table.h"
#include "log.h"



#define TABLE_MIN	1024
//#define TABLE_MIN	16



static unsigned int table_size;



static void to_head(list **root, list *lp);
static list *find_list(list **table, uint8_t *eth_addr);



list **init_table(unsigned int size) // hash table size
{
	table_size = size % UINT_MAX;
	if (table_size < TABLE_MIN) table_size = TABLE_MIN;
	unsigned int mem_size = table_size * sizeof(list *);
	list **table = (list **)malloc(mem_size);
	if (table == NULL) log_pexit("malloc");
	memset(table, (int)NULL, mem_size);

	return table;
}



static void to_head(list **root, list *lp)
{
	list *head = *root;
	if ( lp != head )
	{
		list *pre = lp->pre;
		pre->next = lp->next;
		if (pre->next != NULL)
			(pre->next)->pre = pre;

		lp->pre = NULL;
		lp->next = head;
		head->pre = lp;
		*root = lp;
	}
}



static list *find_list(list **table, uint8_t *eth_addr)
{
	unsigned int key = *((unsigned int *)eth_addr) % table_size;
	list **root = table + key;
	list *p = *root;

	for ( ; p != NULL; p = p->next)
	{
		mac_tbl *mac_t = p->data;
		uint8_t *eth_p = mac_t->hw_addr;
		if (cmp_mac(eth_p, eth_addr) == 0)
		{
			to_head(root, p);
			return p;
		}
	}

	return NULL;
}



mac_tbl *find_data(list **table, uint8_t *eth)
{
	list *p = find_list(table, eth);
	if (p != NULL)
		return p->data;

	return NULL;
}



void add_data(list **table, uint8_t *hw_addr, uint32_t vtep_addr)
{
	mac_tbl *mt;
	list *lp = find_list(table, hw_addr);
	unsigned int key = *((unsigned int *)hw_addr) % table_size;
	list **root = table + key;
	list *head = *root;
//printf("VNI: %02X%02X:%02X%02X:%02X%02X\n", hw_addr[0], hw_addr[1], hw_addr[2],
//		hw_addr[3], hw_addr[4], hw_addr[5]);

	if (lp == NULL)		// Target MAC is not stored
	{
		*root = (list *)malloc(sizeof(list));
		if (*root == NULL) log_pexit("malloc");

		if (head != NULL)
		{
			(*root)->next = head;
			head->pre = *root;
		}

		head = *root;
		head->pre = NULL;
		head->data = (mac_tbl *)malloc(sizeof(mac_tbl));
		if (head->data == NULL) log_pexit("malloc");

		mt = head->data;
		memcpy(mt->hw_addr, hw_addr, MAC_LEN);
		mt->vtep_addr = vtep_addr;
		mt->time = time(NULL);

		head = *root;
		printf("key: %d\n", key);
		printf("head: %p\n", *root);
		printf("hw[0]: %02X\n", (head->data)->hw_addr[0]);
	}
	else			// Target MAC exists
	{
		mt = lp->data;
		mt->vtep_addr = vtep_addr;
		memcpy(mt->hw_addr, hw_addr, MAC_LEN);

		if ( lp != head )
			to_head(root, lp);

//		{
//			list *pre = lp->pre;
//			pre->next = lp->next;
//			if (pre->next != NULL)
//				(pre->next)->pre = pre;
//
//			lp->pre = NULL;
//			lp->next = head;
//			head->pre = lp;
//			*root = lp;
//		}
	}
}



void del_data(list **table, unsigned int key)
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

void show_table(list **table)
{
	list **tp = table;
	list *lp;

	int i = 0;
	int cnt = 0;

	for( ; i < table_size; i++, tp++)
	{
		if (*tp == NULL) continue;
		printf("%3d: ", i);
		for (lp = *tp; lp != NULL; lp = lp->next)
		{
			uint8_t *addr = (lp->data)->hw_addr;
			printf("%02X%02X:%02X%02X:%02X%02X => %"PRIu32",  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], (lp->data)->vtep_addr);
			cnt++;
		}
		printf("NULL\n");
	}

	printf("Count: %d\n", cnt);
}

#endif
