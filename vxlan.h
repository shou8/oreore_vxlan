#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <netinet/in.h>
#include <pthread.h>

#include "base.h"
#include "table.h"




#define VNI_BIT		24
#define VNI_BYTE	3
#define NUMOF_UINT8 (UINT8_MAX + 1)



typedef struct _vxlan_instance_ {

	pthread_t th;
	uint8_t vni[VNI_BYTE];
	device tap;
	list **table;
} vxlan_i;




typedef struct _vxland {
	int port;
	int usoc;
	uint32_t mcast_addr;
	char if_name[IF_NAME_LEN];
	vxlan_i ****vxi;
	char udom[DEFAULT_BUFLEN];
} vxland;



//extern vxi ****vxlan;
extern vxland vxlan;

void init_vxlan(void);
void destroy_vxlan(void);
//void create_vxi(char *buf, uint8_t *vni, char *addr, pthread_t th);
vxlan_i *add_vxi(char *buf, uint8_t *vni);
void del_vxi(char *buf, uint8_t *vni);



#endif /* VXLAN_H_INCLUDED */

