#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>

#include "vxlan.h"



typedef struct _table_
{
	mac_tbl data;
	mac_tbl *next;
} TABLE;



void *init_table(int size);



#endif /* UTIL_H_INCLUDED */

