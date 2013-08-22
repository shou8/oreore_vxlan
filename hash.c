#include <stdlib.h>

#include "log.h"



hash_tbl htbl;

void *init_table(int size)
{
	if ((p = (hash_tbl *)malloc(sizeof(hash_tbl) * size)) == NULL)
		log_pxit("malloc");

	memset(p, 0, size * total);
}



