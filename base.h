#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED



#include <stdint.h>
#include <netinet/in.h>



#define DAEMON_NAME     "vxland"
#define CONTROLLER_NAME "vxconfig"
#define TAP_BASE_NAME   "vxlan"

#define VXLAN_PRODUCT_VERSION   "2.0"
#define CLIENT_VERSION          "2.0"

/* MAC Address Length */
#define MAC_LEN_BITS    48
#define MAC_LEN         6       // 48bits / uint8_t = 6

#define IF_NAME_LEN     256
#define DEFAULT_BUFLEN  256

#define UNIX_DOMAIN_LEN         1024
#define DEFAULT_UNIX_DOMAIN     "/var/run/vxlan.sock"
//#define DEFAULT_MCAST_ADDR        0x640000e0
//
#define DEFAULT_MCAST_ADDR4     "224.0.0.100"
#define DEFAULT_MCAST_ADDR6     "FF15::1"
#define VXLAN_PORT              "4789"

#define DEFAULT_MAC_TIMEOUT     14400
#define DEFAULT_CONFIG_PATH     "/etc/vxlan.conf"
#define DEFAULT_PID_FILE        "/var/run/vxland.pid"

#define EPOLL_MAX_EVENTS 4096



typedef struct _device_ {
    int sock;
    char name[IF_NAME_LEN];
    uint8_t hwaddr[MAC_LEN];
} device;



enum status {
    SUCCESS,
    NOSUCHCMD,
    CMD_FAILED,
    SRV_FAILED,
    LOGIC_FAILED,
    CMD_HELP
};



#endif /* BASE_H_INCLUDED */

