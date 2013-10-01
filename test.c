#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "net.h"
#include "vxlan.h"



//void test_mpool(void);
void test_table(void);
void test_net(void);
void test_vxlan(void);
void test_vxlan_table(void);



void test(void)
{
//	test_mpool();
//	test_table();
//	test_vxlan();
	test_vxlan_table();
	test_net();
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
	list **table = init_table(16);

	uint8_t hw[6];
	hw[0] = 0;
	hw[1] = 0;
	hw[2] = 0;
	hw[3] = 0;
	hw[4] = 0;
	hw[5] = 0;

	add_data(table, hw, 0);
//	show_table();

	hw[0] = 1;
	add_data(table, hw, 1);
//	show_table();

	hw[0] = 2;
	add_data(table, hw, 2);
//	show_table();

	hw[0] = 8;
	add_data(table, hw, 8);
//	show_table();

	hw[0] = 3;
	add_data(table, hw, 3);
//	show_table();

	hw[0] = 4;
	add_data(table, hw, 4);

	hw[0] = 5;
	add_data(table, hw, 5);

	hw[0] = 16;
	add_data(table, hw, 16);

	hw[0] = 24;
	add_data(table, hw, 24);

//	show_table(table);

	hw[0] = 0;
	add_data(table, hw, 10);

	hw[0] = 32;
	add_data(table, hw, 32);

	hw[0] = 0;
	add_data(table, hw, 100);

//	show_table(table);

	hw[0] = 0;
	add_data(table, hw, 0);

//	show_table(table);

//	del_data(table, 1);
//	show_table(table);

//	del_data(table, 1);
//	show_table(table);

//	del_data(table, 0);
	show_table(table);
}



void test_net(void)
{
	int usoc = init_udp_sock();
	outer_loop(usoc);
}



void test_vxlan(void)
{

	uint8_t vni[3];
	memset(vni, 0, 3);
	vxlan = init_vxlan();
	add_vxi(vni);

	vni[2] = 1;
	add_vxi(vni);

	vni[2] = 2;
	add_vxi(vni);

	vni[2] = 3;
	add_vxi(vni);

	vni[0] = 1;
	vni[1] = 1;
	vni[2] = 1;
	add_vxi(vni);

	del_vxi(vni);
	del_vxi(vni);

	vni[0] = UINT8_MAX;
	vni[1] = UINT8_MAX;
	vni[2] = UINT8_MAX;
	add_vxi(vni);

	vni[0] = 1;
	vni[1] = 0;
	vni[2] = 0;
	add_vxi(vni);

	uint8_t hw[6];
	hw[0] = 0x11;
	hw[1] = 0x11;
	hw[2] = 0x11;
	hw[3] = 0x11;
	hw[4] = 0x11;
	hw[5] = 0x11;

	vxi *v = vxlan[1][0][0];
	add_data(v->table, hw, 1);
//	show_table(v->table);

//	show_vxi();

}



void test_vxlan_table(void)
{
	init_vxlan();
	uint8_t vni[3];
	uint8_t hw[6];

	memset(vni, 0, 3);
	memset(hw, 0, 6);

	vni[0] = 1;
	add_vxi(vni);
	vxi *v = vxlan[1][0][0];

	add_data(v->table, hw, 0); 

	hw[0] = 0x01;
	add_data(v->table, hw, 1); 

	hw[0] = 0x11;
	hw[1] = 0x11;
	hw[2] = 0x11;
	hw[3] = 0x11;
	hw[4] = 0x11;
	hw[5] = 0x11;


	add_data(v->table, hw, 1111); 
	show_table(v->table);
}
