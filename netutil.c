#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "netutil.h"



/* ARP Caching time */
#define ARP_CACHE_TIME_HOUR		0
#define ARP_CACHE_TIME_MIN		10	// Linux(Debian 7.0 wheezy 32bit)'s default is 7 min (See Note1)
#define ARP_CACHE_TIME_SEC		0
#define ARP_CACHE_TIME			(((ARP_CACHE_TIME_HOUR * 60) + \
									ARP_CACHE_TIME_MIN) * 60 + \
									ARP_CACHE_TIME_SEC)

/* MAC Address Table */
//#define MAC_ADDR_TABLE_SIZE		(8 * 1024)

/* To represent MAC Address by uint64_t */
#define MAC_SPACE 2		// uint64_t - uint8_t * 6 = 16bit = 2Byte 



/* For Debug: MAC address to String */
void mtos(char *str, uint8_t hwaddr[MAC_LEN] )
{
	sprintf(str, "%x:%x:%x:%x:%x:%x", hwaddr[0], hwaddr[1],
								hwaddr[2], hwaddr[3],
								hwaddr[4], hwaddr[5]);
}



uint8_t cmp_mac( uint8_t hwaddr1[MAC_LEN], uint8_t hwaddr2[MAC_LEN] )
{
	return memcmp(hwaddr1, hwaddr2, MAC_LEN);
}



#ifdef DEBUG

#include <stdlib.h>

/*
 * Get random mac address for DEBUG
 */
void get_mac( uint8_t hwaddr[MAC_LEN] )
{
	int i = 0;
	for (i=0; i<MAC_LEN; i++)
		hwaddr[i] = rand() % 0xFF;
}

#endif



