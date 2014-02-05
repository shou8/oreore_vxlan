#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <getopt.h>
#include <errno.h>
#include <arpa/inet.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "net.h"
#include "sock.h"
#include "ctl.h"



#define CTL_BUF_LEN DEFAULT_BUFLEN * 16



void *inner_loop_thread(void *args);
int order_parse(char *rbuf, char *wbuf);
char *get_argv0(char *buf, char *argv0);
int cmd_usage(char *buf, int cmd_i);
int cmd_usage_all(char **buf);

static void _show_vxi(char *buf);
static void _show_table(char *buf, list **table);

/* Comands: cmd_XXX(char *, int, int, char **); */
int cmd_add_vxi(char *buf, int cmd_i, int argc, char *argv[]);
int cmd_del_vxi(char *buf, int cmd_i, int argc, char *argv[]);
int cmd_exit(char *buf, int cmd_i, int argc, char *argv[]);
int cmd_list(char *buf, int cmd_i, int argc, char *argv[]);
int cmd_mac(char *buf, int cmd_i, int argc, char *argv[]);
int cmd_show(char *buf, int cmd_i, int argc, char *argv[]);
int cmd_help(char *buf, int cmd_i, int argc, char *argv[]);



static char argv0[CTL_BUF_LEN];

struct cmd_entry cmd_t[] = {
	{ "add", cmd_add_vxi, "<VNI>", "Create instance and interface (ex. add 100 => vxlan100)."},
	{ "del", cmd_del_vxi, "<VNI>", "Delete instance and interface."},
	{ "exit", cmd_exit, NULL, "Exit process."},
	{ "list", cmd_list, NULL, "Show instances"},
	{ "mac", cmd_mac, "<VNI>", "Show MAC address table"},
	{ "show", cmd_show, "{instance|mac <VNI>}", "Show instance table (=list) or MAC address table (=mac)."},
	{ "help", cmd_help, NULL, "Show this help message." },
};

int cmd_len = sizeof(cmd_t) / sizeof(struct cmd_entry);



void ctl_loop(char *dom) {

	int usoc, asoc, len;
	char rbuf[CTL_BUF_LEN];
	char wbuf[CTL_BUF_LEN];
	int status = 0;
	char *rp = NULL;

	if ((usoc = init_unix_sock(dom, UNIX_SOCK_SERVER)) < 0)
		log_pcexit("ctl_loop.init_unix_sock");

	while (1) {

		memset(wbuf, 0, sizeof(wbuf));

		if ((asoc = accept(usoc, NULL, 0)) < 0) {
			log_perr("ctl_loop.accept");
			continue;
		}

		if ((len = read(asoc, rbuf, CTL_BUF_LEN)) < 0) {
			log_perr("ctl_loop.read");
			continue;
		}

		rbuf[len] = '\0';
		rp = get_argv0(rbuf, argv0);
		status = order_parse(rp, wbuf);
		switch (status) {
			case SUCCESS:
				break;
			case NOSUCHCMD:
				snprintf(wbuf, CTL_BUF_LEN, "No such command: %s\n", rp);
				break;
			case CMD_FAILED:
				break;
			case SRV_FAILED:
				break;
			default:
				break;
		}

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
	inner_loop(vxlan.vxi[vni[0]][vni[1]][vni[2]]);

	/* Cannot Reach */
	log_crit("The instance (VNI: %"PRIx32") is doen.", To32ex(vni));
	del_vxi(NULL, vni);

	return NULL;
}



char *get_argv0(char *buf, char *argv0) {

	int i, optind;
	char *p = strchr(buf, ' ');
	*(p++) = '\0';
	optind = atoi(buf);

	char *s = p;
	for (i=0; i<optind; i++, p++) {
		p = strchr(p, ' ');
	}
	*(p - 1) = '\0';
	strncpy(argv0, s, CTL_BUF_LEN);

	return p;
}



int order_parse(char *rbuf, char *wbuf) {

	int i, argc;
	char *p;
	char *argv[CTL_BUF_LEN];

	p = strtok(rbuf, " ");
	for (i = 0; i < cmd_len; i++)
		if (str_cmp(cmd_t[i].name, p)) break;

	if (i == cmd_len) return NOSUCHCMD;

	for (argc = 0; p != NULL; argc++) {
		argv[argc] = p;
		p = strtok(NULL, " ");
	}

	return ((cmd_t[i].exec)(wbuf, i, argc, argv));
}



int cmd_usage(char *buf, int cmd_i) {

	if (strlen(argv0) < 1) strncpy(argv0, CONTROLLER_NAME, CTL_BUF_LEN);
	snprintf(buf, CTL_BUF_LEN, "Usage: \"%s\" %s %s\n", argv0, cmd_t[cmd_i].name, (cmd_t[cmd_i].arg == NULL) ? "":cmd_t[cmd_i].arg);
	return CMD_FAILED;
}



/****************
 ****************
 ***          ***
 *** Commands ***
 ***          ***
 ****************
 ****************/



int cmd_add_vxi(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 2)
		return cmd_usage(buf, cmd_i);

	char *vni_s = argv[1];
	uint8_t vni[VNI_BYTE];
	uint32_t vni32 = 0;
	int status = get32and8arr(buf, vni_s, &vni32, vni);
	if (status != SUCCESS) return status;

	if (vxlan.vxi[vni[0]][vni[1]][vni[2]] != NULL) {
		snprintf(buf, CTL_BUF_LEN, "Instance (VNI: %s) has already existed.\n", vni_s);
		return SRV_FAILED;
	}

	vxlan_i *v = add_vxi(buf, vni);
	if (v == NULL) {
		snprintf(buf, CTL_BUF_LEN, "error is occured in server, please refer \"syslog\".\n");
		return SRV_FAILED;
	}

	pthread_t th;
	pthread_create(&th, NULL, inner_loop_thread, vni);
	v->th = th;

	snprintf(buf, CTL_BUF_LEN, "=== Set ===\nVNI\t\t: %"PRIu32"\n", vni32);
	printf("%s", buf);

	return SUCCESS;
}



int cmd_del_vxi(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 2)
		return cmd_usage(buf, cmd_i);

	char *vni_s = argv[1];
	uint32_t vni32 = 0;
	uint8_t vni[VNI_BYTE];
	int status = get32and8arr(buf, vni_s, &vni32, vni);
	if (status != SUCCESS) return status;

	if (vxlan.vxi[vni[0]][vni[1]][vni[2]] == NULL) {
		snprintf(buf, CTL_BUF_LEN, "VNI: %"PRIu32" does not exist.\n", vni32);
		return SRV_FAILED;
	}

	pthread_cancel((vxlan.vxi[vni[0]][vni[1]][vni[2]])->th);
	del_vxi(buf, vni);

	snprintf(buf, CTL_BUF_LEN, "=== Unset ===\nVNI\t\t: %"PRIu32"\n", vni32);
	printf("%s", buf);

	return SUCCESS;
}



int cmd_exit(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 1)
		return cmd_usage(buf, cmd_i);

	uint32_t i, j, k;

	for (i=0; i<NUMOF_UINT8; i++) {
		for (j=0; j<NUMOF_UINT8; j++) {
			for (k=0; k<NUMOF_UINT8; k++) {
				if (vxlan.vxi[i][j][k] != NULL) {
					vxlan_i *v = vxlan.vxi[i][j][k];
					pthread_cancel(v->th);

					uint8_t vni[3] = {i, j, k};
					del_vxi(NULL, vni);
				}
			}
		}
	}

	destroy_vxlan();
	log_iexit("Exit by order.");
	return SUCCESS;
}



int cmd_list(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 1)
		return cmd_usage(buf, cmd_i);

	char *p = buf;
	p = pad_str(p, "  ----- show instances -----  \n");
	_show_vxi(p);

	return SUCCESS;
}



int cmd_mac(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 2)
		return cmd_usage(buf, cmd_i);

	int status = SUCCESS;
	uint8_t vni[VNI_BYTE];
	uint32_t vni32 = 0;
	char *p = buf;

	p = pad_str(p, "  ----- show MAC table -----  \n");
	status = get32and8arr(buf, argv[1], &vni32, vni);
	if (status != SUCCESS) return status;

	if (vxlan.vxi[vni[0]][vni[1]][vni[2]] == NULL) {
		snprintf(buf, CTL_BUF_LEN, "VNI: %"PRIu32" does not exist.\n", vni32);
		return SRV_FAILED;
	}

	_show_table(p, vxlan.vxi[vni[0]][vni[1]][vni[2]]->table);

	return SUCCESS;
}



int cmd_show(char *buf, int cmd_i, int argc, char *argv[]) {

	char *cmd[] = {
		"instance",
		"mac"
	};

	if (argc < 2)
		return cmd_usage(buf, cmd_i);

	int i;
	int len = sizeof(cmd) / sizeof(*cmd);

	for (i=0; i<len; i++)
		if (str_cmp(cmd[i], argv[1])) break;

	switch (i) {
		case 0: {
				char *cargv[] = {argv[0], "list"};
				return cmd_list(buf, cmd_i, argc-1, cargv);
			}
			break;
		case 1:
			return cmd_mac(buf, cmd_i, argc-1, &argv[1]);
			break;
		default:
			return cmd_usage(buf, cmd_i);
	}

	return SUCCESS;
}



int cmd_help(char *buf, int cmd_i, int argc, char *argv[]) {

	int i;
	char *p = buf;
	char str[CTL_BUF_LEN];

	snprintf(str, CTL_BUF_LEN, "\n%17s|%18s| %s\n", "name ", "arguments ", "comment");
	p = pad_str(p, str);
	p = pad_str(p, "   --------------+------------------+---------------\n");

	for (i=0; i<cmd_len; i++) {
		snprintf(str, CTL_BUF_LEN, "%16s %18s : %s\n", cmd_t[i].name, (cmd_t[i].arg == NULL)? "":cmd_t[i].arg, cmd_t[i].comment);
		p = pad_str(p, str);
	}
	p = pad_str(p, "\n");

	return SUCCESS;
}



static void _show_vxi(char *buf) {

	vxlan_i ****vxi = vxlan.vxi;
	char str[DEFAULT_BUFLEN];
	char *p = buf;

	int i,j,k;
	for (i=0; i<NUMOF_UINT8; i++)
		for (j=0; j<NUMOF_UINT8; j++)
			for (k=0; k<NUMOF_UINT8; k++)
				if (vxi[i][j][k] != NULL) {
					uint32_t vni32 = To32(i, j, k);
					snprintf(str, DEFAULT_BUFLEN, "%11"PRIu32" : 0x%06X\n", vni32, vni32);
					p = pad_str(p, str);
				}
}



static void _show_table(char *buf, list **table) {

	int i = 0;
	int cnt = 0;
	char *p = buf;
	char str[DEFAULT_BUFLEN];
	unsigned int table_size = get_table_size();

	list **tp = table;
	list *lp;

	for ( ; i < table_size; i++, tp++) {
		if (*tp == NULL) continue;
		snprintf(str, DEFAULT_BUFLEN, "%7d: ", i);
		p = pad_str(p, str);

		for (lp = *tp; lp != NULL; lp = lp->next) {
			uint8_t *hwaddr = (lp->data)->hw_addr;
			snprintf(str, DEFAULT_BUFLEN, "%02X%02X:%02X%02X:%02X%02X => %s, ", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5], inet_ntoa((lp->data)->vtep_addr));
			p = pad_str(p, str);
			cnt++;
		}
		p = pad_str(p, "NULL\n");
	}

	snprintf(str, DEFAULT_BUFLEN, "\nCount: %d\n", cnt);
	p = pad_str(p, str);
}

