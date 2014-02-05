#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "base.h"
#include "util.h"
#include "sock.h"
#include "net.h"
#include "log.h"
#include "ctl.h"



#define CLIENT_VERSION	"1.0"
#define CTL_BUF_LEN DEFAULT_BUFLEN * 16

#define STDOUT 1



void client_usage(char *bin);
void usage(char *bin);
int argv_to1str_w(char *buf, int optind, int argc, char **argv);



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
	char rbuf[CTL_BUF_LEN]; // To read buffer
	char wbuf[CTL_BUF_LEN]; // To write buffer
	int len;

	disable_syslog();

	opterr = 0;
	if (argc == 1) client_usage(argv[0]);

	while ((opt = getopt_long(argc, argv, "s:hdv", options, NULL)) != -1) {
		switch (opt) {
			case 's':
				dom = optarg;
				continue;
			case 'v':
				fprintf(stderr, "VXLAN client version "CLIENT_VERSION"\n");
				exit(EXIT_SUCCESS);
			case 'h':
				client_usage(argv[0]);
				break;
			default :
				break;
		}
		break;
	}

	if ((sock = init_unix_sock(dom, UNIX_SOCK_CLIENT)) < 0)
		exit(EXIT_FAILURE);

	len = argv_to1str_w(wbuf, optind, argc, argv);

	switch (len) {
		case 0:
		case -1:
			fprintf(stderr, "ERROR: Nothing to be done (No operation).\n\n");
			client_usage(argv[0]);
		case -2:
			fprintf(stderr, "ERROR: Too long arguments (Over %d characters).\n\n", CTL_BUF_LEN);
			client_usage(argv[0]);
	}

	write(sock, wbuf, len);
	len = read(sock, rbuf, CTL_BUF_LEN);
	write(STDOUT, rbuf, len);

    return 0;
}



void client_usage(char *bin) {

	usage(bin);
	fprintf(stderr, "%6s  Use help command: %s help\n", "", bin);
	exit(EXIT_SUCCESS);
}



void usage(char *bin) {

	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s [-h|-v] [-s <socket path>] COMMAND\n", bin);
	fprintf(stderr, "\n");
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, "%10s,    --help: Show this usage.\n", "-h");
	fprintf(stderr, "%10s, --version: Show software version.\n", "-v");
	fprintf(stderr, "%10s,  --socket: socket path.\n", "-s");
	fprintf(stderr, "COMMAND:\n");
}



int argv_to1str_w(char *buf, int optind, int argc, char **argv) {

	int len;
	int rlen;

	snprintf(buf, CTL_BUF_LEN, "%d", optind);
	len = strlen(buf);
	buf[len] = ' ';
	len++;

	rlen = argv_to1str(buf + len, optind, argc, argv);
	if (rlen < 0) return rlen;
	return rlen + len;
}



