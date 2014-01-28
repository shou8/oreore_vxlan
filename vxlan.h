#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <netinet/in.h>
#include <pthread.h>

#include "base.h"
#include "table.h"




#define VNI_BIT		24
#define VNI_BYTE	3



typedef struct _vxlan_instance_ {

	pthread_t th;
	uint8_t vni[VNI_BYTE];
	uint32_t mcast_addr;
	device tap;
	list **table;
} vxi;




extern vxi ****vxlan;

vxi ****init_vxlan(void);
//void destroy_vxlan(void);
//void create_vxi(char *buf, uint8_t *vni, char *addr, pthread_t th);
vxi *add_vxi(char *buf, uint8_t *vni, char *addr);
void del_vxi(char *buf, uint8_t *vni);
void show_vxi(void);



#endif /* VXLAN_H_INCLUDED */

