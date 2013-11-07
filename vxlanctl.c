#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "sock.h"
#include "net.h"
#include "log.h"



int main(int argc, char *argv[]){

	int sock = init_unix_sock(DEFAULT_UNIXSOCK_PATH);
	printf("%d\n", sock);

    return 0;
}

