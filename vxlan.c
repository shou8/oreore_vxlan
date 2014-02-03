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



//vxlan_i ****vxlan;
vxland vxlan = {
	-1,
	-1,
	DEFAULT_MCAST_ADDR,
	"eth0",
	NULL
};



//static int cmp_vni(uint8_t *vni1, uint8_t *vni2);
static device create_vxlan_if(uint8_t *vni);




/*
 * Create 3 Demention Matrix
 */
void init_vxlan(void) {

	vxlan.vxi = (vxlan_i ****)malloc(sizeof(vxlan_i ***) * VXI_MAX);
	if (vxlan.vxi == NULL) log_pcexit("malloc");
	vxlan.vxi[0] = (vxlan_i ***)malloc(sizeof(vxlan_i **) * VXI_MAX * VXI_MAX);
	if (vxlan.vxi[0] == NULL) log_pcexit("malloc");
	vxlan.vxi[0][0] = (vxlan_i **)malloc(sizeof(vxlan_i *) * VXI_MAX * VXI_MAX * VXI_MAX);
	if ( vxlan.vxi[0][0] == NULL ) log_pcexit("malloc");

	int i,j;
	for (i=0; i<VXI_MAX; i++) {
		vxlan.vxi[i] = vxlan.vxi[0] + i * VXI_MAX;
		for (j=0; j<VXI_MAX; j++)
			vxlan.vxi[i][j] = vxlan.vxi[0][0] + i * VXI_MAX * VXI_MAX + j * VXI_MAX;
	}

	memset(vxlan.vxi[0][0], 0, sizeof(vxlan_i *) * VXI_MAX * VXI_MAX * VXI_MAX);
}



void destroy_vxlan(void) {

	free(vxlan.vxi[0][0]);
	free(vxlan.vxi[0]);
	free(vxlan.vxi);
	vxlan.vxi = NULL;
}



static device create_vxlan_if(uint8_t *vni) {

	device tap;
	uint32_t vni32 = To32ex(vni);

	snprintf(tap.name, IF_NAME_LEN, "vxlan%"PRIu32, vni32);
	log_debug("VNI: %"PRIu8".%"PRIu8".%"PRIu8"\n", vni[0], vni[1], vni[2], vni32);
	log_info("Tap interface \"%s\" is created (VNI: %"PRIu32").\n", tap.name, vni32);

//	tap_alloc(tap.name);
//	tap_up(tap.name);
//	tap.sock = init_raw_sock(tap.name);
	tap.sock = tap_alloc(tap.name);
	tap_up(tap.name);
	get_mac(tap.sock, tap.name, tap.hwaddr);

	return tap;
}



vxlan_i *add_vxi(char *buf, uint8_t *vni) {

	vxlan_i *v = (vxlan_i *)malloc(sizeof(vxlan_i));
	if (v == NULL) log_pcexit("malloc");
	memcpy(v->vni, vni, VNI_BYTE);
	v->table = init_table(TABLE_SIZE);
	v->tap = create_vxlan_if(vni);

	vxlan.vxi[vni[0]][vni[1]][vni[2]] = v;

	return v;
}



void del_vxi(char *buf, uint8_t *vni) {

	free(vxlan.vxi[vni[0]][vni[1]][vni[2]]);
	vxlan.vxi[vni[0]][vni[1]][vni[2]] = NULL;
}



void show_vxi(void) {

	vxlan_i ****vxi = vxlan.vxi;

	int i,j,k;
	for (i=0; i<VXI_MAX; i++)
		for (j=0; j<VXI_MAX; j++)
			for (k=0; k<VXI_MAX; k++)
				if (vxi[i][j][k] != NULL) {
					uint32_t vni32 = To32(i, j, k);
					printf("vxlan[%02X][%02X][%02X]: 0x%06X: %p\n", i, j, k, vni32, vxi[i][j][k]);
				}
}



