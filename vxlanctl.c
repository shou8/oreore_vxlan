#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "base.h"
#include "sock.h"
#include "net.h"
#include "log.h"



#define CLIENT_VERSION	"1.0"
#define CTL_BUF_LEN LOG_LINELEN



void usage(char *bin);
int argv_to1str(char *buf, int argc, char **argv);



static struct option options[] = {
	{"socket", required_argument, NULL, 's'},
#ifdef DEBUG
	{"Debug", no_argument, NULL, 'D'},
#endif
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};



int main(int argc, char *argv[]) {

	int opt, sock;
	extern int optind, opterr;
	extern char *optarg;
	char *dom = NULL;
	char wbuf[CTL_BUF_LEN];
	char rbuf[CTL_BUF_LEN];
	int len;

	/* test begin */
/*
sock = init_unix_sock(dom, UNIX_SOCK_CLIENT);
len = argv_to1str(wbuf, argc-1, argv+1);
write(sock, wbuf, len);
len = read(sock, rbuf, CTL_BUF_LEN);
write(1, rbuf, len);
return 0;
*/
	/* test fin */

	disable_syslog();

	opterr = 0;
	if (argc == 1)
		usage(argv[0]);

	while((opt = getopt_long(argc, argv, "s:hdv", options, NULL)) != -1) {
		switch(opt) {
			case 's':
				dom = optarg;
				continue;
			case 'v':
				fprintf(stderr, "Version: "CLIENT_VERSION".\n");
				exit(EXIT_SUCCESS);
			case 'h':
				usage(argv[0]);
				break;
			default :
				break;
		}
		break;
	}

	if ((sock = init_unix_sock(dom, UNIX_SOCK_CLIENT)) < 0)
		exit(EXIT_FAILURE);

	len = argv_to1str(wbuf, argc-1, argv+1);
	switch (len) {
		case 0:
		case -1:
			fprintf(stderr, "ERROR: Nothing to be done (No operation).\n\n");
			usage(argv[0]);
		case -2:
			fprintf(stderr, "ERROR: Too long arguments (Over %d characters).\n\n", CTL_BUF_LEN);
			usage(argv[0]);
	}

	write(sock, wbuf, len);
	len = read(sock, rbuf, CTL_BUF_LEN);
	write(1, rbuf, len);

    return 0;
}



void usage(char *bin) {
	fprintf(stderr, "Usage: %s [-h|-v] [-s <socket path>] COMMAND\n", bin);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "		-h,    --help: Show this usage.\n");
	fprintf(stderr, "		-v, --version: Show software version.\n");
	fprintf(stderr, "		-s,  --socket: socket path.\n");
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}



int argv_to1str(char *buf, int argc, char **argv) {

	int i, len;
	char *p = buf;

	if (argc < 1) return -1;

	for (i=0; i<argc; i++) {
		if (p - buf > CTL_BUF_LEN) return -2;
		len = strlen(argv[i]);
		strncpy(p, argv[i], len);
		p[len] = ' ';
		p += len + 1;
	}
	*(--p) = '\0';

	return (int)(p - buf);
}




