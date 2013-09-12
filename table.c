#include <stdlib.h>

#include "mpool.h"
#include "log.h"
#include "table.h"



static mpool_t *pool;
static TABLE *table;
static int table_size;



void *init_table(int size) // hash table size
{
	int mem_size = size * sizeof(TABLE);
	table_size = size;

	if ((pool = mp_create(mem_size)) == NULL)
		log_exit("Memory allocation initializing error: mp_create");

	table = (TABLE *)mp_alloc(mem_size, pool);

	return table;
}



TABLE_DATA_TYPE *find_table(int tbl_key)
{
	int key = tbl_key % table_size;

	return NULL;
}
