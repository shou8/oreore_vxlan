#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <malloc.h>
//#include <memory.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "iftap.h"
#include "sock.h"
#include "net.h"
#include "netutil.h"



#define VXI_MAX (UINT8_MAX + 1)



vxi ****vxlan;
//static int cmp_vni(uint8_t *vni1, uint8_t *vni2);
static device create_vxlan_if(uint8_t *vni);



/*
 * Create 3 Demention Matrix
 */
vxi ****init_vxlan(void) {

	vxlan = (vxi ****)malloc(sizeof(vxi ***) * VXI_MAX);
	if (vxlan == NULL) log_pcexit("malloc");
	vxlan[0] = (vxi ***)malloc(sizeof(vxi **) * VXI_MAX * VXI_MAX);
	if (vxlan[0] == NULL) log_pcexit("malloc");
	vxlan[0][0] = (vxi **)malloc(sizeof(vxi *) * VXI_MAX * VXI_MAX * VXI_MAX);
	if ( vxlan[0][0] == NULL ) log_pcexit("malloc");

	int i,j;
	for (i=0; i<VXI_MAX; i++) {
		vxlan[i] = vxlan[0] + i * VXI_MAX;
		for (j=0; j<VXI_MAX; j++)
			vxlan[i][j] = vxlan[0][0] + i * VXI_MAX * VXI_MAX + j * VXI_MAX;
	}

	memset(vxlan[0][0], 0, sizeof(vxi *) * VXI_MAX * VXI_MAX * VXI_MAX);

	return vxlan;
}



void destroy_vxlan(void) {

	free(vxlan[0][0]);
	free(vxlan[0]);
	free(vxlan);
}



static device create_vxlan_if(uint8_t *vni) {

	device tap;
	uint32_t vni32 = To32ex(vni);

	snprintf(tap.name, IF_NAME_LEN, "vxlan%"PRIu32, vni32);
	log_debug("VNI: %"PRIu8".%"PRIu8".%"PRIu8"\n", vni[0], vni[1], vni[2], vni32);
	log_info("Tap interface \"%s\" is created (VNI: %"PRIu32").\n", tap.name, vni32);

	tap_alloc(tap.name);
	tap_up(tap.name);
	tap.sock = init_raw_sock(tap.name);
	get_mac(tap.sock, tap.name, tap.hwaddr);

	return tap;
}



vxi *add_vxi(char *buf, uint8_t *vni, char *addr, char *if_name) {

	vxi *v = (vxi *)malloc(sizeof(vxi));
	if (v == NULL) log_pcexit("malloc");
	memcpy(v->vni, vni, VNI_BYTE);
	v->table = init_table(TABLE_SIZE);
	v->tap = create_vxlan_if(vni);

	if (inet_pton(AF_INET, addr, &v->mcast_addr) != 1) {
		log_berr(buf, "inet_pton: Cannot translate (%s).\n", addr);
		free(v);
		return NULL;
	}

	if (join_mcast_group(get_usoc(), 0, addr, if_name) < 0) {
		log_berr(buf, "Cannot initialize socket (%d).\n", get_usoc());
		log_bperr(buf, "socket");
		free(v);
		return NULL;
	}

	vxlan[vni[0]][vni[1]][vni[2]] = v;

	return v;
}



void del_vxi(char *buf, uint8_t *vni) {

	vxi *v = vxlan[vni[0]][vni[1]][vni[2]];
	if (leave_mcast_group(get_usoc(), v->mcast_addr, (v->tap).name) < 0) {
		log_berr(buf, "Cannot initialize socket (%d).\n", get_usoc());
		log_bperr(buf, "socket");
		free(v);
	}

	free(vxlan[vni[0]][vni[1]][vni[2]]);
	vxlan[vni[0]][vni[1]][vni[2]] = NULL;
}



void show_vxi(void) {

	int i,j,k;
	for (i=0; i<VXI_MAX; i++)
		for (j=0; j<VXI_MAX; j++)
			for (k=0; k<VXI_MAX; k++)
				if (vxlan[i][j][k] != NULL) {
					uint32_t vni32 = To32(i, j, k);
					printf("vxlan[%02X][%02X][%02X]: 0x%06X: %p\n", i, j, k, vni32, vxlan[i][j][k]);
				}
}



