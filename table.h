#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>

#include "vxlan.h"



typedef struct _list_
{
	mac_tbl *data;
	struct _LIST_ *next;
} LIST;



void *init_table(int size);
mac_tbl *find_data(int tbl_key, uint8_t *data);



#endif /* UTIL_H_INCLUDED */

