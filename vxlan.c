#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "iftap.h"
#include "sock.h"
#include "net.h"
#include "netutil.h"



/* Written on vxlan.h

typedef struct _vxland {
	int port;
	int usoc;
	struct in_addr mcast_addr;
	char if_name[IF_NAME_LEN];
	vxlan_i ****vxi;
	char udom[DEFAULT_BUFLEN];
	int lock;
} vxland;
*/
vxland vxlan = {
	-1,
	-1,
	{ DEFAULT_MCAST_ADDR },
	"eth0",
	NULL,
	DEFAULT_UNIX_DOMAIN,
	0
};



static device create_vxlan_if(uint8_t *vni);




/*
 * Create 3 Demention Matrix
 */
void init_vxlan(void) {

	vxlan.vxi = (vxlan_i ****)malloc(sizeof(vxlan_i ***) * NUMOF_UINT8);
	if (vxlan.vxi == NULL) log_pcexit("malloc");
	vxlan.vxi[0] = (vxlan_i ***)malloc(sizeof(vxlan_i **) * NUMOF_UINT8 * NUMOF_UINT8);
	if (vxlan.vxi[0] == NULL) log_pcexit("malloc");
	vxlan.vxi[0][0] = (vxlan_i **)malloc(sizeof(vxlan_i *) * NUMOF_UINT8 * NUMOF_UINT8 * NUMOF_UINT8);
	if ( vxlan.vxi[0][0] == NULL ) log_pcexit("malloc");

	int i,j;
	for (i=0; i<NUMOF_UINT8; i++) {
		vxlan.vxi[i] = vxlan.vxi[0] + i * NUMOF_UINT8;
		for (j=0; j<NUMOF_UINT8; j++)
			vxlan.vxi[i][j] = vxlan.vxi[0][0] + i * NUMOF_UINT8 * NUMOF_UINT8 + j * NUMOF_UINT8;
	}

	memset(vxlan.vxi[0][0], 0, sizeof(vxlan_i *) * NUMOF_UINT8 * NUMOF_UINT8 * NUMOF_UINT8);
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

	close(vxlan.vxi[vni[0]][vni[1]][vni[2]]->tap.sock);
	free(vxlan.vxi[vni[0]][vni[1]][vni[2]]);
	vxlan.vxi[vni[0]][vni[1]][vni[2]] = NULL;
}



