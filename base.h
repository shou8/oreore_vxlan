#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED



#include <stdint.h>



#define DAEMON_NAME		"vxland"
#define TAP_BASE_NAME	"vxlan"

/* MAC Address Length */
#define MAC_LEN_BITS	48
#define MAC_LEN			6		// 48bits / uint8_t = 6

#define IF_NAME_LEN		256


typedef struct _device_
{
	int sock;
	char name[IF_NAME_LEN];
	uint8_t hwaddr[MAC_LEN];
} device;



#endif /* BASE_H_INCLUDED */

