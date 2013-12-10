#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "base.h"
#include "log.h"
#include "net.h"
#include "vxlan.h"
#include "ctl.h"

#ifdef DEBUG
#include "test.h"
#endif /* DEBUG */



int main(int argc, char *argv[]) {

#ifdef DEBUG
	enable_debug();
//	test();
#endif /* DEUBG */

	vxlan = init_vxlan();	// Global variable (declared in "vxlan.c")

	pthread_t oth;		// outer_loop thread
	pthread_create(&oth, NULL, outer_loop_thread, (void *)NULL);

#ifdef DEBUG
//	test();
#endif /* DEUBG */

	ctl_loop(NULL); /* TODO */

    return 0;
}




