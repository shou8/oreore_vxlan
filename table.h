#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdlib.h>

#include "vxlan.h"



typedef struct _table_
{
	TABLE_DATA_TYPE data;
	TABLE_DATA_TYPE *next;
} TABLE;



/**
 * Compare data method.
 * You must define.
 *
 * + Policy
 *
 *		 Return int: 0	: equal
 *				   other: Not equal
 *
 */
extern int cmp_data(TABLE_DATA_TYPE *data1, TABLE_DATA_TYPE *data2);

void *init_table(int size);



#endif /* UTIL_H_INCLUDED */

