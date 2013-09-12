#include <stdio.h>
#include "mpool.h"



void test(void)
{
	printf("sizeof(type)\n");
	printf("sizeof(int): %d\n", sizeof(int));
	printf("sizeof(char): %d\n", sizeof(char));

	void *ti = malloc(sizeof(int));
	void *tc = malloc(sizeof(char));
	printf("Manually malloced\n");
	printf("sizeof(int): %d\n", sizeof(*ti));
	printf("sizeof(char): %d\n", sizeof(*tc));

	mpool_t *pool = mp_create(sizeof(int));
	int *ip = (int *)mp_alloc(sizeof(int), pool);
	char *cp = (char *)mp_alloc(sizeof(char), pool);
	printf("Automatically malloced\n");
	printf("sizeof(int): %d\n", sizeof(*ip));
	printf("sizeof(char): %d\n", sizeof(*cp));

	printf("Invalid value malloced\n");
	int *i = (int *)mp_alloc(-1, pool);
	printf("sizeof(i): %d\n", sizeof(*i));

	*ip = 0;
	*cp = 'a';

	printf("%d\n", *ip);
	printf("%c\n", *cp);

	mp_alloc(sizeof(char) * 1000, pool);

	mp_destroy(pool);

}

