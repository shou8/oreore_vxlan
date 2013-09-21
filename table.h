#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>

#include "vxlan.h"



#define TABLE_SIZE 4096



void *init_table(int size);
mac_tbl *find_data(uint8_t *eth_addr);
void add_data(uint8_t *hw_addr, uint32_t vtep_addr);



#ifdef DEBUG
void show_table(void);
#endif



#endif /* UTIL_H_INCLUDED */

