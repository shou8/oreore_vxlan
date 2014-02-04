#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

#include "base.h"
#include "log.h"
#include "net.h"
#include "vxlan.h"
#include "ctl.h"

#ifdef DEBUG
#include "test.h"
#endif /* DEBUG */



void *outer_loop_thread(void *args);



static struct option options[] = {
	{0, 0, 0, 0}
};



int main(int argc, char *argv[]) {

	int opt;
	extern int optind, opterr;
	extern char *optarg;

/*
#ifdef DEBUG
enable_debug();
#else
disable_debug();
#endif
*/

	init_vxlan();
	pthread_t oth;
	pthread_create(&oth, NULL, outer_loop_thread, (void *)NULL);
	ctl_loop(NULL);

    return 0;
}



void *outer_loop_thread(void *args) {

	outer_loop();

	return NULL;
}



