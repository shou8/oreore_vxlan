#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdlib.h>

#include "vxlan.h"



typedef struct _hash_tbl_
{
	HASH_DATA_TYPE data;
	HASH_DATA_TYPE *next;
} hash_tbl;



void *init_table(int hash_size);



#endif /* UTIL_H_INCLUDED */

