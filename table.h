#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>

#include "vxlan.h"



typedef struct _table_
{
	TABLE_DATA_TYPE data;
	TABLE_DATA_TYPE *next;
} TABLE;



void *init_table(int size);



#endif /* UTIL_H_INCLUDED */

