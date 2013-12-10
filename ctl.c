#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "base.h"
#include "log.h"
#include "vxlan.h"
#include "net.h"
#include "sock.h"



#define CTL_BUF_LEN	64



// Inner Loop Argument
typedef struct _ilt_arg_ {
	uint8_t *vni;
	char *addr;
} ilt_arg;



void *inner_loop_thread(void *args);
void order_parse(int sock, char *buf);



void ctl_loop(char *dom) {

	int usoc, asoc, len;
	char buf[CTL_BUF_LEN];

	if ((usoc = init_unix_sock(dom, UNIX_SOCK_SERVER)) < 0)
		log_pcexit("ctl_loop.init_unix_sock");

	while(1) {

		if ((asoc = accept(usoc, NULL, 0)) < 0) {
			log_perr("ctl_loop.accept");
			continue;
		}

		if ((len = read(asoc, buf, CTL_BUF_LEN)) < 0) {
			log_perr("ctl_loop.read");
			continue;
		}

		printf("%d\n", len);
		buf[len] = '\0';
		order_parse(asoc, buf);
	}
}



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

	del_vxi(a->vni);

	return NULL;
}



void *outer_loop_thread(void *args) {

	outer_loop();

	return NULL;
}



void order_parse(int sock, char *buf) {

	char *p;

	printf("ORIGINAL: %s\n", buf);
	p = strtok(buf, " ");
	while (p != NULL) {
//	while ((p = strtok(NULL, "\t"))) {
		printf("%s\n", p);
		p = strtok(NULL, " ");
	}
}
