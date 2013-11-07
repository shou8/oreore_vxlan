#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "base.h"
#include "log.h"
#include "vxlan.h"
#include "net.h"
#include "sock.h"



#define CTL_BUF_LEN	64



void ctl_loop(void) {

	int sock, len;
	char buf[CTL_BUF_LEN];

	if ((sock = init_unix_sock(DEFAULT_UNIXSOCK_PATH)) < 0)
		return;

	while(1) {
		if ((len = read(sock, buf, CTL_BUF_LEN)) < 0) {
			log_perr("read");
			continue;
		}

		printf("%s\n", buf);
	}
}
