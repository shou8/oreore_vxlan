#include <stdlib.h>

#include "log.h"

void *init_hash(int size)
{
	void *p;

	if ((p = malloc(sizeof(size))) == NULL)
		log_pxit("malloc");

	memset(p, 0, size * total);
}
