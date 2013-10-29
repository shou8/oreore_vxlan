#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "base.h"
#include "log.h"
#include "net.h"
#include "vxlan.h"

#ifdef DEBUG
#include "test.h"
#endif /* DEBUG */



void *inner_loop_thread(void *args);
void *outer_loop_thread(void *args);



// Inner Loop Argument
typedef struct _ilt_arg_ {
	uint8_t *vni;
	char *addr;
} ilt_arg;



int main(int argc, char *argv[]) {

#ifdef DEBUG
	enable_debug();
//	test();
#endif /* DEUBG */

	vxlan = init_vxlan();	// Global variable (declared in "vxlan.c")

	pthread_t oth;		// outer_loop thread
	pthread_create(&oth, NULL, outer_loop_thread, (void *)NULL);

#ifdef DEBUG
	test();
#endif /* DEUBG */

    return 0;
}



void *inner_loop_thread(void *args) {

	ilt_arg *a = (ilt_arg *)args;

	create_vxi(a->vni, a->addr, pthread_self());
	inner_loop(vxlan[a->vni[0]][a->vni[1]][a->vni[2]]);

	/*
	 * "inner_loop" function infinite loop.
	 * So we don'nt have to care memory location.
	 *
	 * If inner_loop is not infinite loop,
	 * you have to use "malloc" to allocate to heap area.
	 */

	return NULL;
}



void *outer_loop_thread(void *args) {

	outer_loop();

	return NULL;
}



