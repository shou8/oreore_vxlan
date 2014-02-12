#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "base.h"
#include "log.h"
#include "net.h"
#include "vxlan.h"
#include "cmd.h"
#include "sock.h"



void *outer_loop_thread(void *args);
void create_pid_file(char *pid_path);
void usage(char *bin);



static struct option options[] = {
	{"daemon", no_argument, NULL, 'd'},
#ifdef DEBUG
	{"Debug", no_argument, NULL, 'D'},
#endif /* DEBUG */
	{"help", no_argument, NULL, 'h'},
	{"interface", required_argument, NULL, 'i'},
	{"multicast", required_argument, NULL, 'm'},
	{"port", required_argument, NULL, 'p'},
	{"pidfile", required_argument, NULL, 'P'},
	{"socket", required_argument, NULL, 's'},
	{"version", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};



int main(int argc, char *argv[]) {

	int opt;
	extern int optind, opterr;
	extern char *optarg;

	int enable_D = 0;
	int enable_d = 0;
	int enable_i = 0;
	int enable_m = 0;

	char pid_path[DEFAULT_BUFLEN] = "/var/run/vxland.pid";

	opterr = 0;
	disable_debug();
	disable_syslog();
	vxlan.mcast_addr.s_addr = DEFAULT_MCAST_ADDR;

	while ((opt = getopt_long(argc, argv, "dDhi:m:p:P:s:v", options, NULL)) != -1) {
		switch (opt) {
			case 'd':
				enable_d = 1;
				break;
#ifdef DEBUG
			case 'D':
				enable_D = 1;
				enable_debug();
				break;
#endif /* DEBUG */
			case 'h':
				usage(argv[0]);
				break;
			case 'i':
				enable_i = 1;
				strncpy(vxlan.if_name, optarg, DEFAULT_BUFLEN);
				break;
			case 'm':
				enable_m = 1;
				if (inet_aton(optarg, &vxlan.mcast_addr) == 0)
					log_cexit("Invalid address: %s\n", optarg);
				break;
			case 'p':
				vxlan.port = atoi(optarg);
				if (vxlan.port == 0) log_cexit("Invalid port number: %s\n", optarg);
				log_info("Port number :%d\n", vxlan.port);
				break;
			case 'P':
				strncpy(pid_path, optarg, DEFAULT_BUFLEN);
				break;
			case 's':
				strncpy(vxlan.udom, optarg, DEFAULT_BUFLEN);
				printf("%s\n", vxlan.udom);
				break;
			case 'v':
				fprintf(stderr, "VXLAN version "VXLAN_PRODUCT_VERSION"\n");
				exit(EXIT_SUCCESS);
				break;
			default:
				usage(argv[0]);
		}
	}

	if ( enable_D && enable_d ) {
		disable_debug();
		enable_syslog();
		log_warn("Incompatible option \"-d\" and \"-D\", \"-d\" has priority over \"-D\".");
		log_warn("Because this process writes many debug information on /dev/null\n");
	}

	if ( ! enable_i ) {
		log_warn("Multicast interface is not set.\n");
		log_warn("Default interface \"%s\" is used\n", vxlan.if_name);
	}

	if ( ! enable_m ) {
		log_warn("Multicast address is not set.\n");
		log_warn("Default address \"%s\" is used\n", inet_ntoa(vxlan.mcast_addr));
	}

	init_vxlan();
	vxlan.usoc = init_udp_sock(vxlan.port);
	if (vxlan.usoc < 0) log_cexit("outer_loop.socket: Bad descripter\n");
	if (join_mcast_group(vxlan.usoc, vxlan.mcast_addr, vxlan.if_name) < 0)
		log_pcexit("socket");

	if ( enable_d ) {
		create_pid_file(pid_path);
		if (daemon(1, 0) != 0) log_perr("daemon");
		disable_syslog();
	}

	pthread_t oth;
	pthread_create(&oth, NULL, outer_loop_thread, (void *)NULL);

	ctl_loop(vxlan.udom);

    return 0;
}



void usage(char *bin) {
	fprintf(stderr, "usage!\n");
	exit(EXIT_SUCCESS);
}



void *outer_loop_thread(void *args) {

	outer_loop();

	return NULL;
}



void create_pid_file(char *pid_path) {

	FILE *fp;
	if ((fp = fopen(pid_path, "w")) == NULL)
		log_pcexit("fopen");
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
}



