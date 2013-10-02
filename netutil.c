#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "netutil.h"
#include "log.h"



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



int cmp_mac( uint8_t hwaddr1[MAC_LEN], uint8_t hwaddr2[MAC_LEN] )
{
	return memcmp(hwaddr1, hwaddr2, MAC_LEN);
}



uint8_t *get_mac(int sock, char *name, uint8_t *hwaddr)
{
	struct ifreq ifreq;
	uint8_t *addr;

	strncpy(ifreq.ifr_name, name, sizeof(ifreq.ifr_name) - 1);
	if ( ioctl(sock, SIOCGIFHWADDR, &ifreq) == -1 )
	{
		log_pcrit("ioctl");
		close(sock);
		return NULL;
	} else {
		addr = (uint8_t *)&ifreq.ifr_hwaddr.sa_data;
		memcpy(hwaddr, addr, MAC_LEN);
	}

	return hwaddr;
}



#ifdef DEBUG

#include <stdlib.h>

/*
 * Get random mac address for DEBUG
 */
void get_ran_mac( uint8_t hwaddr[MAC_LEN] )
{
	int i = 0;
	for (i=0; i<MAC_LEN; i++)
		hwaddr[i] = rand() % 0xFF;
}



char *eth_ntoa(uint8_t *hwaddr, char *buf, size_t size)
{
	snprintf(buf, size, "%02X:%02X:%02X:%02X:%02X:%02X", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	return buf;
}



void print_eth_h(struct ether_header *eh, FILE *fp)
{
	char buf[128];

	fprintf(fp, "ether_header -----\n");
	fprintf(fp, "ether_dhost: %s\n", eth_ntoa(eh->ether_dhost, buf, sizeof(buf)));
	fprintf(fp, "ether_shost: %s\n", eth_ntoa(eh->ether_shost, buf, sizeof(buf)));
	fprintf(fp, "ether_type : %02X", ntohs(eh->ether_type));

	switch(eh->ether_type)
	{
		case ETH_P_IP:
			fprintf(fp, "(IP)\n");
			break;
		case ETH_P_IPV6:
			fprintf(fp, "(IPv6)\n");
			break;
		case ETH_P_ARP:
			fprintf(fp, "(ARP)\n");
			break;
		default:
			fprintf(fp, "(unknown)\n");
			break;
	}
}

#endif



