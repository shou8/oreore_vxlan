#include <stdio.h>

#include "table.h"



//void test_mpool(void);
void test_table(void);



void test(void)
{
//	test_mpool();
	test_table();
}



/*
void test_mpool(void)
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
*/



void test_table(void)
{
//	init_vxlan(1);
	init_table(8);

	uint8_t hw[6];
	hw[0] = 0;
	hw[1] = 0;
	hw[2] = 0;
	hw[3] = 0;
	hw[4] = 0;
	hw[5] = 0;

	add_data(hw, 0);
//	show_table();

	hw[0] = 1;
	add_data(hw, 1);
//	show_table();

	hw[0] = 2;
	add_data(hw, 2);
//	show_table();

	hw[0] = 8;
	add_data(hw, 8);
//	show_table();

	hw[0] = 3;
	add_data(hw, 3);
//	show_table();

	hw[0] = 4;
	add_data(hw, 4);

	hw[0] = 5;
	add_data(hw, 5);

	hw[0] = 16;
	add_data(hw, 16);

	hw[0] = 24;
	add_data(hw, 24);

	show_table();

	hw[0] = 0;
	add_data(hw, 10);

	hw[0] = 32;
	add_data(hw, 32);

	hw[0] = 0;
	add_data(hw, 100);

	show_table();

	hw[0] = 0;
	add_data(hw, 0);

	show_table();

	del_data(1);
	show_table();

	del_data(1);
	show_table();

	del_data(0);
	show_table();
}
