#include <stdlib.h>

#include "mpool.h"
#include "log.h"



static mpool_t *pool;
static hash_tbl *htbl;



void *init_table(int hash_size)
{
	int mem_size = hash_size * sizeof(hash_tbl);

	if ((pool = mp_create(mem_size)) == NULL)
		log_exit("Memory allocation initializing error: mp_create");

	htbl = (hash_tbl *)mp_alloc(mem_size, pool);
}



