#ifndef NETUTIL_H_INCLUDED
#define NETUTIL_H_INCLUDED


#include <stdint.h>

#include "base.h"



extern void mtos(char *str, uint8_t hwaddr[MAC_LEN]);
extern uint8_t cmp_mac(uint8_t hwaddr1[MAC_LEN], uint8_t hwaddr2[MAC_LEN]);

#ifdef DEBUG
extern void get_mac( uint8_t hwaddr[MAC_LEN]);
#endif



#endif /* NETUTIL_H_INCLUDED */

/*
 *** Note1: ARP cache time ***
 *
 * Minimum of ARP cache time is 1 sec in Linux.
 * Actually, it is written in /proc/sys/net/ipv4/neigh/ethX/locktime.
 * But, I really measured 7 mins on Debian.
 * You can 
 *
 */
