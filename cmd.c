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
#include "cmd.h"



#define CTL_BUF_LEN DEFAULT_BUFLEN * 16



struct cmd_entry {
	const char *name;
	int (*exec)(char *buf, int cmd_i, int argc, char *argv[]);
	const char *arg;
	const char *comment;
};



void *inner_loop_thread(void *args);
int order_parse(char *rbuf, char *wbuf);
char *get_argv0(char *buf, char *argv0);
int cmd_usage(char *buf, int cmd_i, char *mes);
int cmd_usage_all(char **buf);

static void _show_vxi(char *buf);
static void _show_table(char *buf, list **table);

/* Comands: cmd_XXX(char *, int, int, char **); */
static int _cmd_add_vxi(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_del_vxi(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_exit(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_list(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_mac(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_show(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_help(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_clear(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_flush(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_set(char *buf, int cmd_i, int argc, char *argv[]);
static int _cmd_info(char *buf, int cmd_i, int argc, char *argv[]);



struct cmd_entry _cmd_t[] = {
	{ "add", _cmd_add_vxi, "<VNI>", "Create instance and interface (ex. add 100 => vxlan100)."},
	{ "del", _cmd_del_vxi, "<VNI>", "Delete instance and interface."},
	{ "exit", _cmd_exit, NULL, "Exit process."},
	{ "list", _cmd_list, NULL, "Show instances"},
	{ "mac", _cmd_mac, "<VNI>", "Show MAC address table"},
	{ "show", _cmd_show, "{instance|mac <VNI>}", "Show instance table (=list) or MAC address table (=mac)."},
	{ "help", _cmd_help, NULL, "Show this help message." },
	{ "clear", _cmd_clear, "[force] <VNI>", "Clear time outed MAC address from table. If added \"force\", similarly behave \"flush\" commnad." },
	{ "flush", _cmd_flush, "<VNI>", "Drop and Create MAC address table (all cached MAC address is discarded)." },
	{ "set", _cmd_set, "{time <cache time>|mac <MAC_addr> <VTEP IPaddr>}", "Set command."},
	{ "info", _cmd_info, NULL, "Get general information."},
};

static int _cmd_len = sizeof(_cmd_t) / sizeof(struct cmd_entry);



void ctl_loop(char *dom) {

	int usoc, asoc, len;
	char rbuf[CTL_BUF_LEN];
	char wbuf[CTL_BUF_LEN];

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
		switch (order_parse(rbuf, wbuf)) {
			case SUCCESS:
				break;
			case NOSUCHCMD:
				snprintf(wbuf, CTL_BUF_LEN, "ERROR, No such command: %s\n", rbuf);
				break;
			case CMD_FAILED:
			case SRV_FAILED:
			case LOGIC_FAILED:
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



int order_parse(char *rbuf, char *wbuf) {

	int i, argc;
	char *p;
	char *argv[CTL_BUF_LEN];

	p = strtok(rbuf, " ");
	for (i = 0; i < _cmd_len; i++)
		if (str_cmp(_cmd_t[i].name, p)) break;

	if (i == _cmd_len) return NOSUCHCMD;

	for (argc = 0; p != NULL; argc++) {
		argv[argc] = p;
		p = strtok(NULL, " ");
	}

	return ((_cmd_t[i].exec)(wbuf, i, argc, argv));
}



int _cmd_usage(char *buf, int cmd_i, char *mes) {

	char *p = buf;
	if (mes != NULL) p = pad_str(p, mes);
	snprintf(p, CTL_BUF_LEN, "Usage: %s %s %s\n", CONTROLLER_NAME, _cmd_t[cmd_i].name, (_cmd_t[cmd_i].arg == NULL) ? "":_cmd_t[cmd_i].arg);
	return CMD_FAILED;
}



/****************
 ****************
 ***          ***
 *** Commands ***
 ***          ***
 ****************
 ****************/



static int _cmd_add_vxi(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc > 3)
		return _cmd_usage(buf, cmd_i, "ERROR: Too many arguments\n");

	char *vni_s = argv[1];
	uint8_t vni[VNI_BYTE];
	uint32_t vni32 = 0;
	int status = get32and8arr(buf, vni_s, &vni32, vni);
	if (status != SUCCESS) return status;

	if (vxlan.vxi[vni[0]][vni[1]][vni[2]] != NULL) {
		snprintf(buf, CTL_BUF_LEN, "ERROR: The instance (VNI: %s) has already existed.\n", vni_s);
		return LOGIC_FAILED;
	}

	vxlan_i *v = add_vxi(buf, vni, (argc == 3) ? argv[2] : NULL);
	if (v == NULL) {
		snprintf(buf, CTL_BUF_LEN, "error is occured in server, please refer \"syslog\".\n");
		return SRV_FAILED;
	}

	pthread_t th;
	pthread_create(&th, NULL, inner_loop_thread, vni);
	v->th = th;

	snprintf(buf, CTL_BUF_LEN, "=== Set ===\nVNI\t\t: %"PRIu32"\n", vni32);
	log_info("VNI: %"PRIu32" is added\n", vni32);

	return SUCCESS;
}



static int _cmd_del_vxi(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 2)
		return _cmd_usage(buf, cmd_i, (argc < 2)? "ERROR: Too few arguments\n":"ERROR: Too many arguments\n");

	char *vni_s = argv[1];
	uint32_t vni32 = 0;
	uint8_t vni[VNI_BYTE];
	int status = get32and8arr(buf, vni_s, &vni32, vni);
	if (status != SUCCESS) return status;

	if (vxlan.vxi[vni[0]][vni[1]][vni[2]] == NULL) {
		snprintf(buf, CTL_BUF_LEN, "ERROR: The instance (VNI: %"PRIu32") does not exist.\n", vni32);
		return LOGIC_FAILED;
	}

	pthread_cancel((vxlan.vxi[vni[0]][vni[1]][vni[2]])->th);
	del_vxi(buf, vni);

	snprintf(buf, CTL_BUF_LEN, "=== Unset ===\nVNI\t\t: %"PRIu32"\n", vni32);
	printf("%s", buf);

	return SUCCESS;
}



static int _cmd_exit(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 1)
		return _cmd_usage(buf, cmd_i, "ERROR: Too many arguments\n");

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
	log_iexit("Exit by order.\n");
	return SUCCESS;
}



static int _cmd_list(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 1)
		return _cmd_usage(buf, cmd_i, "ERROR: Too many arguments\n");

	char *p = buf;
	p = pad_str(p, "  ----- show instances -----  \n");
	_show_vxi(p);

	return SUCCESS;
}



static int _cmd_mac(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc != 2)
		return _cmd_usage(buf, cmd_i, (argc < 2)?"Too few arguments\n":"Too many arguments\n");

	int status = SUCCESS;
	uint8_t vni[VNI_BYTE];
	uint32_t vni32 = 0;
	char *p = buf;

	p = pad_str(p, "  ----- show MAC table -----  \n");
	status = get32and8arr(buf, argv[1], &vni32, vni);
	if (status != SUCCESS) return status;

	if (vxlan.vxi[vni[0]][vni[1]][vni[2]] == NULL) {
		snprintf(buf, CTL_BUF_LEN, "ERROR: The instance (VNI: %"PRIu32") does not exist.\n", vni32);
		return LOGIC_FAILED;
	}

	_show_table(p, vxlan.vxi[vni[0]][vni[1]][vni[2]]->table);

	return SUCCESS;
}



static int _cmd_show(char *buf, int cmd_i, int argc, char *argv[]) {

	char *cmd[] = {
		"instance",
		"mac"
	};

	if (argc < 2)
		return _cmd_usage(buf, cmd_i, NULL);

	int i;
	int len = sizeof(cmd) / sizeof(*cmd);

	for (i=0; i<len; i++)
		if (str_cmp(cmd[i], argv[1])) break;

	switch (i) {
		case 0: {
				char *cargv[] = {argv[0], "list"};
				return _cmd_list(buf, cmd_i, argc-1, cargv);
			}
			break;
		case 1:
			return _cmd_mac(buf, cmd_i, argc-1, &argv[1]);
			break;
		default:
			return _cmd_usage(buf, cmd_i, NULL);
	}

	return SUCCESS;
}



static int _cmd_help(char *buf, int cmd_i, int argc, char *argv[]) {

	int i;
	char *p = buf;
	char str[CTL_BUF_LEN];

	if (argc != 1)
		return _cmd_usage(buf, cmd_i, "ERROR: Too many arguments\n");

	snprintf(str, CTL_BUF_LEN, "\n%17s|%18s| %s\n", "name ", "arguments ", "comment");
	p = pad_str(p, str);
	p = pad_str(p, "   --------------+------------------+---------------\n");

	for (i=0; i<_cmd_len; i++) {
		snprintf(str, CTL_BUF_LEN, "%16s %18s : %s\n", _cmd_t[i].name, (_cmd_t[i].arg == NULL)? "":_cmd_t[i].arg, _cmd_t[i].comment);
		p = pad_str(p, str);
	}
	p = pad_str(p, "\n");

	return CMD_HELP;
}



static int _cmd_clear(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc < 2)
		return _cmd_usage(buf, cmd_i, "ERROR: Too few arguments\n");

	int num = (argc == 3 && str_cmp(argv[1], "force")) ? 2 : 1;
	char *vni_s = argv[num];
	uint8_t vni[VNI_BYTE];
	uint32_t vni32 = 0;
	int status = get32and8arr(buf, vni_s, &vni32, vni);
	if (status != SUCCESS) return status;

	vxlan_i *vi = vxlan.vxi[vni[0]][vni[1]][vni[2]];
	if (vi == NULL) {
		snprintf(buf, CTL_BUF_LEN, "ERROR: The instance (VNI: %"PRIu32") does not exist.\n", vni32);
		return LOGIC_FAILED;
	}

	if (num == 2) {
		vi->table = clear_table_all(vi->table);
		snprintf(buf, CTL_BUF_LEN, "INFO : The instance(VNI: %"PRIu32")'s MAC address table is recreated.\n", vni32);
	} else {
		int num = clear_table_timeout(vi->table, vxlan.timeout);
		snprintf(buf, CTL_BUF_LEN, "INFO : Timeouted MAC address (%d) were deleted from VNI %"PRIu32"\n", num, vni32);
	}

	return SUCCESS;
}



static int _cmd_flush(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc < 2)
		return _cmd_usage(buf, cmd_i, "ERROR: Too few arguments\n");

	char *argv_w[] = {"clear", "force", argv[1]};
	return _cmd_clear(buf, cmd_i, 3, argv_w);
}



static int _cmd_set(char *buf, int cmd_i, int argc, char *argv[]) {

	if (argc < 2)
		return _cmd_usage(buf, cmd_i, "ERROR: Too few arguments\n");

	char *cmd[] = {
		"time",
		"mac"
	};

	int i = 0;
	int len = sizeof(cmd) / sizeof(*cmd);

	for (i=0; i<len; i++)
		if (str_cmp(cmd[i], argv[1])) break;

	switch (i) {
		case 0:
			if (argc != 3) return _cmd_usage(buf, cmd_i, "ERROR: Too many arguments\n");
			int time = atoi(argv[2]);
			if (time <= 0) {
				snprintf(buf, CTL_BUF_LEN, "ERROR: Cannot convert value of timeout: %s\n", argv[2]);
				return LOGIC_FAILED;
			}
			vxlan.timeout = time;
			snprintf(buf, CTL_BUF_LEN, "MAC address table's cache timeout is set: %d\n", time);
			break;
		case 1:
			break;
		default:
			return _cmd_usage(buf, cmd_i, "ERROR: Too few arguments\n");
	}

	return SUCCESS;
}



static int _cmd_info(char *buf, int cmd_i, int argc, char *argv[]) {

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

	for (i=0 ; i < table_size; i++, tp++) {
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



