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
#include "config.h"



void *outer_loop_thread(void *args);
void create_pid_file(char *pid_path);
void usage(char *bin);



static struct option options[] = {
	{"config", no_argument, NULL, 'c'},
	{"daemon", no_argument, NULL, 'd'},
	{"Debug", no_argument, NULL, 'D'},
	{"help", no_argument, NULL, 'h'},
	{"interface", required_argument, NULL, 'i'},
	{"multicast_addr", required_argument, NULL, 'm'},
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

	int enable_c = 0;
	int enable_d = 0;
	int enable_D = 0;
	int enable_i = 0;
	int enable_m = 0;
	int enable_p = 0;
	int enable_P = 0;
	int enable_s = 0;

	char pid_path[DEFAULT_BUFLEN] = DEFAULT_PID_FILE;

	opterr = 0;
	disable_debug();
	disable_syslog();

#ifdef DEBUG
	enable_debug();
#endif

	while ((opt = getopt_long(argc, argv, "c:dDhi:m:p:P:s:v", options, NULL)) != -1) {
		switch (opt) {
			case 'c':
				enable_c = optind-1;
				break;
			case 'd':
				enable_d = 1;
				break;
#ifdef DEBUG
			case 'D':
				enable_D = 1;
				break;
#endif
			case 'h':
				usage(argv[0]);
				break;
			case 'i':
				enable_i = optind-1;
				break;
			case 'm':
				enable_m = optind-1;
				break;
			case 'p':
				enable_p = optind-1;
				break;
			case 'P':
				enable_P = optind-1;
				break;
			case 's':
				enable_s = optind-1;
				break;
			case 'v':
				fprintf(stderr, "VXLAN version "VXLAN_PRODUCT_VERSION"\n");
				exit(EXIT_SUCCESS);
				break;
			default:
				usage(argv[0]);
		}
	}

	/* Get configuration */
	if ( enable_c != 0 ) strncpy(vxlan.conf_path, argv[enable_c], DEFAULT_BUFLEN);
	struct config conf[DEFAULT_BUFLEN];
	int len = get_config(vxlan.conf_path, conf);

	/* Set config paramaters (before option parameter) */
	int i;
	if (len > 0) {
		for (i=0; i<len; i++) {
			if (conf[i].param_no == 4) continue;
			if (set_config(&conf[i]) < 0) log_cexit("Invalid configuration\n");
		}
	}

	if ( enable_D != 0 ) enable_debug();
	if ( enable_i != 0 ) strncpy(vxlan.if_name, argv[enable_i], DEFAULT_BUFLEN);
	if ( enable_m != 0 )
		strncpy(vxlan.cmaddr, argv[enable_m], DEFAULT_BUFLEN);
	else
		log_info("Using default multicast address: %s\n", vxlan.cmaddr);
	if ( enable_p != 0 ) strncpy(vxlan.port, argv[enable_p], DEFAULT_BUFLEN);
	if ( enable_P != 0 ) strncpy(pid_path, argv[enable_P], DEFAULT_BUFLEN);
	if ( enable_s != 0 ) strncpy(vxlan.udom, optarg, DEFAULT_BUFLEN);

	if ( enable_d && enable_D ) {
		disable_debug();
		enable_syslog();
		log_warn("Incompatible option \"-d\" and \"-D\", \"-d\" has priority over \"-D\".");
		log_warn("Because this process writes many debug information on /dev/null\n");
	}

	/* Start vxlan process */
	if (init_vxlan() < 0)
		log_cexit("Init vxlan failed\n");

	/* Daemon */
	if ( enable_d ) {
		if (daemon(1, 0) != 0) log_perr("daemon");
		create_pid_file(pid_path);
		disable_syslog();
	}

	/* Set parameter (After option) */
	if (len > 0) {
		for (i=0; i<len; i++) {
			if (conf[i].param_no != 4) continue;
			if (set_config(&conf[i]) < 0) log_cexit("Invalid configuration\n");
		}
	}

	pthread_t oth;
	pthread_create(&oth, NULL, outer_loop_thread, (void *)NULL);

	ctl_loop(vxlan.udom);

    return 0;
}


void usage(char *bin) {
	fprintf(stderr, "Usage: %s [OPTIONS]\n", bin);
	fprintf(stderr, "\n");
	fprintf(stderr, "OPTIONS: \n");
	fprintf(stderr, "        -d                       : Enable daemon mode\n");
#ifdef DEBUG
	fprintf(stderr, "        -D                       : Enable debug mode\n");
#endif
	fprintf(stderr, "        -h                       : Show this help\n");
	fprintf(stderr, "        -i <interface>           : Set multicast interface\n");
	fprintf(stderr, "        -m <multicast address>   : Set multicast address\n");
	fprintf(stderr, "        -p <port number>         : Set port number\n");
	fprintf(stderr, "        -P <Path to PID file>    : Set path to PID file\n");
	fprintf(stderr, "        -s <Path to Unix Socket> : Set path to unix socket\n");
	fprintf(stderr, "        -v                       : Show version\n");
	fprintf(stderr, "\n");
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



