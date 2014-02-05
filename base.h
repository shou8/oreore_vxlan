#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED



#include <stdint.h>
#include <netinet/in.h>



#define DAEMON_NAME		"vxland"
#define CONTROLLER_NAME	"vxlanctl"
#define TAP_BASE_NAME	"vxlan"

#define VXLAN_PRODUCT_VERSION	"1.0"
#define CLIENT_VERSION			"1.0"

/* MAC Address Length */
#define MAC_LEN_BITS	48
#define MAC_LEN			6		// 48bits / uint8_t = 6

#define IF_NAME_LEN		256
#define DEFAULT_BUFLEN	256

#define UNIX_DOMAIN_LEN			1024
#define DEFAULT_UNIX_DOMAIN		"/var/run/vxlan.sock"
#define DEFAULT_MCAST_ADDR	0x010000e0



typedef struct _device_ {
	int sock;
	char name[IF_NAME_LEN];
	uint8_t hwaddr[MAC_LEN];
} device;



enum status {
	SUCCESS,
	NOSUCHCMD,
	CMD_FAILED,
	SRV_FAILED
};



#endif /* BASE_H_INCLUDED */

