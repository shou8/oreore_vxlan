#ifndef VXLAN_H_INCLUDED
#define VXLAN_H_INCLUDED



#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "table.h"




#define VNI_BIT     24
#define VNI_BYTE    3
#define NUMOF_UINT8 (UINT8_MAX + 1)



typedef struct _vxlan_instance_ {

    pthread_t th;
    uint8_t vni[VNI_BYTE];
    device tap;
    list **table;
    struct sockaddr_storage maddr;
    int timeout;                // Specific
} vxlan_i;



typedef struct _vxland {

    int usoc;
    int epfd;
    char port[DEFAULT_BUFLEN];

    sa_family_t family;
    char cmaddr[DEFAULT_BUFLEN];
    struct sockaddr_storage maddr;

    char *if_name;
    vxlan_i ****vxi;
    char udom[DEFAULT_BUFLEN];
    int lock;
    int timeout;                // Default
    char conf_path[DEFAULT_BUFLEN];
} vxland;



//extern vxi ****vxlan;
extern vxland vxlan;

int init_vxlan(void);
void destroy_vxlan(void);
vxlan_i *add_vxi(char *buf, uint8_t *vni, struct sockaddr_storage);
void del_vxi(char *buf, uint8_t *vni);



#endif /* VXLAN_H_INCLUDED */

