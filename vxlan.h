#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <netinet/in.h>
#include <pthread.h>

#include "base.h"
#include "table.h"




#define VNI_BIT		24
#define VNI_BYTE	3

#define To32(x, y, z) ((x) << 16 | (y) << 8 | (z))
#define To32ex(arr) (To32((arr[0]), (arr[1]), (arr[2])))



typedef struct _vxlan_instance_ {

	pthread_t th;
	uint8_t vni[VNI_BYTE];
	uint32_t mcast_addr;
	int usoc;
	device tap;
	list **table;
} vxi;




extern vxi ****vxlan;

vxi ****init_vxlan(void);
void destroy_vxlan(void);
vxi *add_vxi(uint8_t *vni, char *addr);
void create_vxi(uint8_t *vni, char *addr, pthread_t th);
void del_vxi(uint8_t *vni);
void destroy_vxi(uint8_t *vni);
void show_vxi(void);



#endif /* VXLAN_H_INCLUDED */

