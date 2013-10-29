#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "base.h"
#include "log.h"
#include "vxlan.h"
#include "iftap.h"
#include "net.h"
#include "netutil.h"



#define VXI_MAX (UINT8_MAX + 1)



vxi ****vxlan;
//static int cmp_vni(uint8_t *vni1, uint8_t *vni2);
static device create_vxlan_if(uint8_t *vni);



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

	memset(vxlan[0][0], (int)NULL, sizeof(vxi *) * VXI_MAX * VXI_MAX * VXI_MAX);

	return vxlan;
}



void destroy_vxlan(void) {

	free(vxlan[0][0]);
	free(vxlan[0]);
	free(vxlan);
}



static device create_vxlan_if(uint8_t *vni) {

	device tap;
	//uint32_t vni32 = vni[0] << 16 | vni[1] << 8 | vni[2];
	uint32_t vni32 = To32ex(vni);

	snprintf(tap.name, IF_NAME_LEN, "vxlan%"PRIo32, vni32);
	log_debug("VNI: %"PRIu8".%"PRIu8".%"PRIu8"\n", vni[0], vni[1], vni[2]);
	log_info("Tap interface \"%s\" is created (VNI: %"PRIo32").\n", tap.name);
	tap.sock = init_raw_sock(tap.name);
	get_mac(tap.sock, tap.name, tap.hwaddr);

	return tap;
}



vxi *add_vxi(uint8_t *vni, char *addr) {

	if (vxlan[vni[0]][vni[1]][vni[2]] != NULL) {
		uint32_t vni32 = To32ex(vni);
		log_err("VNI: %"PRIo32" has already exist\n", vni32);
		return NULL;
	}

	vxi *v = (vxi *)malloc(sizeof(vxi));
	if (v == NULL) log_pcexit("malloc");
	memcpy(v->vni, vni, VNI_BYTE);
	v->table = init_table(TABLE_SIZE);
	v->tap = create_vxlan_if(vni);
	v->usoc = join_mcast_group(get_usoc(), 0, addr, NULL);

	if (inet_pton(AF_INET, addr, &v->mcast_addr) != 1) {
		log_err("inet_pton: Cannot translate (%s).", addr);
		free(v);
		return NULL;
	}

	if (v->usoc == -1) {
		log_err("Cannot initialize socket (%d)", v->usoc);
		log_perr("socket");
		free(v);
		return NULL;
	}

	vxlan[vni[0]][vni[1]][vni[2]] = v;

	return v;
}



void create_vxi(uint8_t *vni, char *addr, pthread_t th) {

	vxi *v = add_vxi(vni, addr);
	if (v != NULL) v->th = th;
}



void del_vxi(uint8_t *vni) {

	if (vxlan[vni[0]][vni[1]][vni[2]] == NULL) {
		uint32_t vni32 = To32ex(vni);
		log_err("VNI: %"PRIo32" does not exist\n", vni32);
		return;
	}

	free(vxlan[vni[0]][vni[1]][vni[2]]);
	vxlan[vni[0]][vni[1]][vni[2]] = NULL;
}



void destroy_vxi(uint8_t *vni) {

	pthread_t th = (vxlan[vni[0]][vni[1]][vni[2]])->th;
	pthread_cancel(th);
	del_vxi(vni);
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



