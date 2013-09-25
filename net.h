#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED



int init_raw_sock(char *dev);
int init_udp_sock(void);
int outer_loop(int sock);



#endif
