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
	device tap;
	list **table;
} vxi;




struct vxland_info {
	uint32_t mcast_addr;
	char if_name[IF_NAME_LEN];
};



extern vxi ****vxlan;
extern struct vxland_info v_info;

vxi ****init_vxlan(void);
//void destroy_vxlan(void);
//void create_vxi(char *buf, uint8_t *vni, char *addr, pthread_t th);
vxi *add_vxi(char *buf, uint8_t *vni);
void del_vxi(char *buf, uint8_t *vni);
void show_vxi(void);



#endif /* VXLAN_H_INCLUDED */

