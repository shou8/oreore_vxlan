#ifndef NETUTIL_H_INCLUDED
#define NETUTIL_H_INCLUDED


#include <inttypes.h>
#include <net/ethernet.h>
#include <sys/socket.h>

#include "base.h"



void mtos(char *str, uint8_t hwaddr[MAC_LEN]);
int cmp_mac( uint8_t hwaddr1[MAC_LEN], uint8_t hwaddr2[MAC_LEN] );
uint8_t *get_mac(int sock, char *name, uint8_t *hwaddr);
struct in_addr get_addr(char *if_name);
int inet_atom(uint8_t mac[MAC_LEN], char *mac_s);
int get_sockaddr(struct sockaddr_storage *saddr, const char *caddr, char *cport);
char *get_straddr(struct sockaddr_storage *saddr);

#ifdef DEBUG
void get_ran_mac( uint8_t hwaddr[MAC_LEN]);
char *eth_ntoa(uint8_t *hwaddr, char *buf, size_t size);
void print_eth_h(struct ether_header *eh, FILE *fp);
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
