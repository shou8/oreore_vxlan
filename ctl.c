#include <stdio.h>
#include <stdlib.h>
#include <unitstd.h>
#include <pthread.h>

#include "base.h"
#include "log.h"
#include "vxlan.h"
#include "net.h"



#define CTL_BUF_LEN	64



// Inner Loop Argument
typedef struct _ilt_arg_ {
	uint8_t *vni;
	char *addr;
} ilt_arg;



void *inner_loop_thread(void *args);



/*
 * "inner_loop" function infinite loop.
 * So we don'nt have to care memory location.
 *
 * If inner_loop is not infinite loop,
 * you have to use "malloc" to allocate to heap area.
 */

void *inner_loop_thread(void *args) {

	ilt_arg *a = (ilt_arg *)args;

	create_vxi(a->vni, a->addr, pthread_self());
	inner_loop(vxlan[a->vni[0]][a->vni[1]][a->vni[2]]);

	del_vni(vni);

	return NULL;
}



void ctl_loop(void) {

	int sock, len;
	char buf[CTL_BUF_LEN];

	sock = init_unix_sock(DEFAULT_UNIXSOCK_PATH);

	while(1) {
		if ((len = read(sock, buf, CTL_BUF_LEN)) < 0) {
			log_perr("read");
			continue;
		}

		printf("%s\n", buf);
	}
}
