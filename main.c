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



struct vxland_info v_info = {
	DEFAULT_MCAST_ADDR,
	"eth0"
};



void *outer_loop_thread(void *args);



int main(int argc, char *argv[]) {

#ifdef DEBUG
enable_debug();
#else
disable_debug();
#endif /* DEUBG */

	vxlan = init_vxlan();	// Global variable (declared in "vxlan.c")

	pthread_t oth;		// outer_loop thread
	pthread_create(&oth, NULL, outer_loop_thread, (void *)NULL);

#ifdef DEBUG
//	test();
#endif /* DEUBG */

	ctl_loop(NULL);

    return 0;
}



void *outer_loop_thread(void *args) {

	outer_loop();

	return NULL;
}



