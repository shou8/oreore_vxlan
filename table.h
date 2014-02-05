#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>

#include "base.h"

#define TABLE_SIZE 4096
//#define TABLE_SIZE 16



typedef struct _mac2ip4_table_
{
	uint8_t hw_addr[MAC_LEN];
	uint32_t vtep_addr;
	time_t time;
} mac_tbl;



typedef struct _list_
{
	mac_tbl *data;
	struct _list_ *pre;
	struct _list_ *next;
} list;



list **init_table(unsigned int size);
mac_tbl *find_data(list **table, uint8_t *eth_addr);
void add_data(list **table, uint8_t *hw_addr, uint32_t vtep_addr);
void del_data(list **table, unsigned int key);
unsigned int get_table_size(void);



#endif /* UTIL_H_INCLUDED */

