#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>

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
int order_parse(int sock, char *buf);

int cmd_add_vxi(char *buf, int argc, char *argv[]);
int cmd_del_vxi(char *buf, int argc, char *argv[]);



struct cmd_entry cmd_t[] = {
	{ "add", cmd_add_vxi },
	{ "del", cmd_del_vxi },
};

int cmd_len = sizeof(cmd_t) / sizeof(struct cmd_entry);


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

printf("%s\n", buf);
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

	uint8_t *vni = (uint8_t *)args;
	inner_loop(vxlan[vni[0]][vni[1]][vni[2]]);
	log_crit("The instance (VNI: %"PRIx32") is doen.", To32ex(vni));
	del_vxi(vni);

	return NULL;
}



int order_parse(int sock, char *buf) {

	int i;
	char *p;

	p = strtok(buf, " ");
	for (i = 0; i < cmd_len; i++)
		if (strncmp(cmd_t[i].name, p, strlen(cmd_t[i].name)) == 0) break;

	if (i == cmd_len) return CMD_FAILED;

	while (p != NULL) {
//	while ((p = strtok(NULL, "\t"))) {
		printf("%s\n", p);
		p = strtok(NULL, " ");
	}

	return SUCCESS;
}



int cmd_add_vxi(char *buf, int argc, char *argv[]) {

	if (argc < 2) {
		strncpy(buf, "add <VNI> <Multicast Address>", CTL_BUF_LEN);
		return CMD_FAILED;
	}

	char *vni_s = argv[0];
	char *addr = argv[1];
	uint8_t vni[VNI_BYTE];

	str2uint8arr(vni_s, vni);
	vxi *v = add_vxi(buf, vni, addr);
	if (v == NULL) {
		strncpy(buf, "error is occured in server, please refer \"syslog\".", CTL_BUF_LEN);
		return SRV_FAILED;
	}

	pthread_t th;
	pthread_create(&th, NULL, inner_loop_thread, vni);
	v->th = th;

	return SUCCESS;
}



int cmd_del_vxi(char *buf, int argc, char *argv[]) {

	if (argc < 1) {
		strncpy(buf, "del <VNI>", CTL_BUF_LEN);
		return CMD_FAILED;
	}

	uint8_t vni[VNI_BYTE];

	if (vxlan[vni[0]][vni[1]][vni[2]] == NULL) {
		uint32_t vni32 = To32ex(vni);
		snprintf(buf, CTL_BUF_LEN, "VNI: %"PRIu32" does not exist.\n", vni32);
		return SRV_FAILED;
	}

	pthread_t th = (vxlan[vni[0]][vni[1]][vni[2]])->th;
	pthread_cancel(th);
	del_vxi(vni);

	return SUCCESS;
}
