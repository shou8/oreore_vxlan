#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "base.h"
#include "util.h"
#include "log.h"
#include "vxlan.h"
#include "tap.h"
#include "sock.h"
#include "net.h"
#include "netutil.h"



vxland vxlan = {
    -1,
    -1,
    VXLAN_PORT,
    AF_INET6,
    DEFAULT_MCAST_ADDR6,
    {},
    NULL,
    NULL,
    DEFAULT_UNIX_DOMAIN,
    0,
    DEFAULT_MAC_TIMEOUT,
    DEFAULT_CONFIG_PATH
};



static void init_vxlan_info(void);
static void init_vxi(void);
static device create_vxlan_if(uint8_t *vni);



int init_vxlan(void) {

    init_vxlan_info();
    init_vxi();
    if ((vxlan.usoc = init_udp_sock(vxlan.family, vxlan.port)) < 0)
        return -1;

    switch (vxlan.family) {
        case AF_INET:
            if (join_mcast4_group(vxlan.usoc, ((struct sockaddr_in *)(&vxlan.maddr))->sin_addr, vxlan.if_name) < 0)
            return -1;
            break;
        case AF_INET6:
            if (join_mcast6_group(vxlan.usoc, ((struct sockaddr_in6 *)(&vxlan.maddr))->sin6_addr, vxlan.if_name) < 0)
            return -1;
            break;
        default:
            log_cexit("Unknown address family\n");
    }

    return 0;
}



void init_vxlan_info(void) {

    in_port_t port = (in_port_t)atoi(vxlan.port);
    if (port == 0) {
        log_warn("Invalid port number: %s\n", vxlan.port);
        log_warn("Using default port number: %d\n", VXLAN_PORT);
        strncpy(vxlan.port, VXLAN_PORT, DEFAULT_BUFLEN);
    }

    if (get_sockaddr(&vxlan.maddr, vxlan.cmaddr, vxlan.port) < 0) {
        log_pcrit("getaddrinfo");
        log_cexit("Invalid multicast address: %s\n", vxlan.cmaddr);
    }

    vxlan.family = vxlan.maddr.ss_family;
}



/*
 * Create 3 Demention Matrix
 */
static void init_vxi(void) {

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
    log_info("Tap interface \"%s\" is created (VNI: %"PRIu32").\n", tap.name, vni32);

    tap.sock = tap_alloc(tap.name);
    if (tap.sock < 0) log_cexit("Cannot create tap interface\n");
    tap_up(tap.name);
    get_mac(tap.sock, tap.name, tap.hwaddr);

    return tap;
}



vxlan_i *add_vxi(char *buf, uint8_t *vni, struct sockaddr_storage maddr) {

    vxlan_i *v = (vxlan_i *)malloc(sizeof(vxlan_i));
    if (v == NULL) {
        log_pcrit("malloc");
        return NULL;
    }

    memcpy(v->vni, vni, VNI_BYTE);
    v->table = init_table(DEFAULT_TABLE_SIZE);
    if (v->table == NULL) {
        log_pcrit("malloc");
        free(v);
        return NULL;
    }

    v->tap = create_vxlan_if(vni);
    v->timeout = vxlan.timeout;
    v->maddr = maddr;

    vxlan.vxi[vni[0]][vni[1]][vni[2]] = v;

    return v;
}



void del_vxi(char *buf, uint8_t *vni) {

    sa_family_t family = vxlan.vxi[vni[0]][vni[1]][vni[2]]->maddr.ss_family;

    if (memcmp(&vxlan.vxi[vni[0]][vni[1]][vni[2]]->maddr, &vxlan.maddr, sizeof(struct sockaddr_storage)) != 0) {
        int i, j, k;
        for (i=0; i<NUMOF_UINT8; i++) {
            for (j=0; j<NUMOF_UINT8; j++) {
                for (k=0; k<NUMOF_UINT8; k++) {
                    if (vxlan.vxi[i][j][k] == NULL) continue;
                    if (memcmp(&vxlan.vxi[i][j][k]->maddr, &vxlan.vxi[vni[0]][vni[1]][vni[2]]->maddr, sizeof(struct sockaddr_storage)) != 0)
                        break;
                }
            }
        }

        if ( i != NUMOF_UINT8 || j != NUMOF_UINT8 || k != NUMOF_UINT8) {
            if (family == AF_INET)
                leave_mcast4_group(vxlan.usoc, ((struct sockaddr_in *)(&vxlan.vxi[vni[0]][vni[1]][vni[2]]->maddr))->sin_addr, vxlan.if_name);
            else
                leave_mcast6_group(vxlan.usoc, ((struct sockaddr_in6 *)(&vxlan.vxi[vni[0]][vni[1]][vni[2]]->maddr))->sin6_addr, vxlan.if_name);
        }
    }

    close(vxlan.vxi[vni[0]][vni[1]][vni[2]]->tap.sock);
    free(vxlan.vxi[vni[0]][vni[1]][vni[2]]);
    vxlan.vxi[vni[0]][vni[1]][vni[2]] = NULL;
}



