#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <getopt.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "net.h"
#include "sock.h"



#define CTL_BUF_LEN	LOG_LINELEN



struct cmd_entry {
	const char *name;
	int (*exec)(char *buf, int argc, char *argv[]);
};



void *inner_loop_thread(void *args);
int order_parse(char *rbuf, char *wbuf);

int cmd_add_vxi(char *buf, int argc, char *argv[]);
int cmd_del_vxi(char *buf, int argc, char *argv[]);



struct cmd_entry cmd_t[] = {
	{ "add", cmd_add_vxi },
	{ "del", cmd_del_vxi },
};

int cmd_len = sizeof(cmd_t) / sizeof(struct cmd_entry);


void ctl_loop(char *dom) {

	int usoc, asoc, len;
	char rbuf[CTL_BUF_LEN];
	char wbuf[CTL_BUF_LEN];

	if ((usoc = init_unix_sock(dom, UNIX_SOCK_SERVER)) < 0)
		log_pcexit("ctl_loop.init_unix_sock");

	while(1) {

		if ((asoc = accept(usoc, NULL, 0)) < 0) {
			log_perr("ctl_loop.accept");
			continue;
		}

		if ((len = read(asoc, rbuf, CTL_BUF_LEN)) < 0) {
			log_perr("ctl_loop.read");
			continue;
		}

		rbuf[len] = '\0';
		order_parse(rbuf, wbuf);
		len = write(asoc, wbuf, strlen(wbuf));
		if (len < 0) log_perr("write");
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

	uint8_t *vni = (uint8_t *)args;
	inner_loop(vxlan[vni[0]][vni[1]][vni[2]]);

	/* Cannot Reach */
	log_crit("The instance (VNI: %"PRIx32") is doen.", To32ex(vni));
	del_vxi(NULL, vni);

	return NULL;
}



int order_parse(char *rbuf, char *wbuf) {

	int i, argc;
	char *p;
	char *argv[CTL_BUF_LEN];

	p = strtok(rbuf, " ");
	for (i = 0; i < cmd_len; i++)
		if (strncmp(cmd_t[i].name, p, strlen(cmd_t[i].name)) == 0) break;

	if (i == cmd_len) return CMD_FAILED;

	for (argc = 0; p != NULL; argc++) {
		argv[argc] = p;
		p = strtok(NULL, " ");
	}

	return ((cmd_t[i].exec)(wbuf, argc, argv));
}



int cmd_add_vxi(char *buf, int argc, char *argv[]) {

	int opt;
	extern int optind;
	extern char *optarg;
	char *vni_s = NULL;
	char *if_name = NULL;
	char *maddr = NULL;
	uint8_t vni[VNI_BYTE];

	struct option addopt[] = {
		{"help", no_argument, NULL, 'h'},
		{"interface", required_argument, NULL, 'i'},
		{"mcastaddr", required_argument, NULL, 'm'},
		{0, 0, 0, 0}
	};

	char usage[] = "add [-i <interface_name>] [-m <Multicast Address>] <VNI>";

	while((opt = getopt_long(argc, argv, "i:m:", addopt, NULL)) != -1) {
		switch(opt) {
			case 'i':
				if_name = optarg;
				break;
			case 'm':
				maddr = optarg;
				break;
			case '?':
				snprintf(buf, CTL_BUF_LEN, "%s\n", usage);
				return CMD_FAILED;
		}
	}

	if (optind >= argc) {
		snprintf(buf, CTL_BUF_LEN, "%s\n", usage);
		return CMD_FAILED;
	}

	vni_s = argv[optind];

	if (str2uint8arr(vni_s, vni) < 0) {
		snprintf(buf, CTL_BUF_LEN, "Invalid VNI: %s\n", vni_s);
		return CMD_FAILED;
	}

	if (vxlan[vni[0]][vni[1]][vni[2]] != NULL) {
		snprintf(buf, CTL_BUF_LEN, "Instance (VNI: %s) has already existed.\n", vni_s);
		return SRV_FAILED;
	}

	vxi *v = add_vxi(buf, vni, maddr, if_name);
	if (v == NULL) {
		snprintf(buf, CTL_BUF_LEN, "error is occured in server, please refer \"syslog\".\n");
		return SRV_FAILED;
	}

	pthread_t th;
	pthread_create(&th, NULL, inner_loop_thread, vni);
	v->th = th;

	snprintf(buf, CTL_BUF_LEN, "=== Set ===\nVNI\t\t: %s\nMCAST ADDR\t: %s\n", vni_s, maddr);
	printf("%s", buf);

	return SUCCESS;
}



int cmd_del_vxi(char *buf, int argc, char *argv[]) {

	if (argc < 2) {
		snprintf(buf, CTL_BUF_LEN, "del <VNI>\n");
		return CMD_FAILED;
	}

	char *vni_s = argv[1];

	uint8_t vni[VNI_BYTE];
	str2uint8arr(vni_s, vni);

	if (vxlan[vni[0]][vni[1]][vni[2]] == NULL) {
		snprintf(buf, CTL_BUF_LEN, "VNI: %s does not exist.\n", vni_s);
		return SRV_FAILED;
	}

	pthread_t th = (vxlan[vni[0]][vni[1]][vni[2]])->th;
	pthread_cancel(th);
	del_vxi(buf, vni);

	return SUCCESS;
}
