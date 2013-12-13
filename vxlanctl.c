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
	extern int optind;
	extern char *optarg;
	char *bin = argv[0];
	char *dom = NULL;

	disable_syslog();

	if (argc == 1)
		usage(bin);

	while((opt = getopt_long(argc, argv, ":s:hadv", options, NULL)) != -1) {
		switch(opt) {
			case 's':
				dom = optarg;
				break;
			case 'v':
				fprintf(stderr, "Version: "CLIENT_VERSION".\n");
				exit(EXIT_SUCCESS);
			case 'h':
				usage(bin);
		}
	}

	if ((sock = init_unix_sock(dom, UNIX_SOCK_CLIENT)) < 0)
		exit(EXIT_FAILURE);

	/* TODO */
	write(sock, argv+1, CTL_BUF_LEN);

/*
#ifdef DEBUG
	char buf[BUFSIZ];
	while (1) {
		printf("===\n");
		read(0, buf, BUFSIZ);
		write(sock, buf, BUFSIZ);
		printf("%s", buf);
		printf("---\n");
		read(sock, buf, BUFSIZ);
		write(1, buf, BUFSIZ);
		printf("---\n");
	}
#endif
*/

    return 0;
}



void usage(char *bin) {
	fprintf(stderr, "Usage: %s [-h|-v] [-s <socket path>]\n", bin);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "		-h,    --help: Show this usage.\n");
	fprintf(stderr, "		-v, --version: Show software version.\n");
	fprintf(stderr, "		-s,  --socket: socket path.\n");
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}
