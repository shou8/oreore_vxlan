#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>
#include <netinet/in.h>



#define TABLE_SIZE 4096
#define DEFAULT_TABLE_SIZE TABLE_SIZE



typedef struct _mac2ip4_table_
{
	uint8_t hw_addr[MAC_LEN];
	struct in_addr vtep_addr;
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
void add_data(list **table, uint8_t *hw_addr, struct in_addr *vtep_addr);
//void del_data(list **table, unsigned int key);
int del_data(list **table, uint8_t *hw_addr, struct in_addr *vtep_addr);
list **clear_table_all(list **table);
int clear_table_timeout(list **table, int cache_time);
unsigned int get_table_size(void);



#endif /* UTIL_H_INCLUDED */

