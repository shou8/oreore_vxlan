#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "base.h"



#define CTL_BUF_LEN DEFAULT_BUFLEN



void split_32to8(uint32_t val, uint8_t *arr) {

	arr[0] = (uint8_t)(val >> 16);
	arr[1] = (uint8_t)(val >> 8);
	arr[2] = (uint8_t)(val);
}



uint32_t str2uint8arr(char *str, uint8_t *arr) {

	errno = 0;
	uint32_t val = strtoul(str, NULL, 0);
	split_32to8(val, arr);
	return val;
}



int str_cmp(const char *p1, const char *p2) {

	int len = strlen(p1);
	return (len == strlen(p2) && strncmp(p1, p2, len) == 0); 
}



int argv_to1str(char *buf, int optind, int argc, char **argv) {

	int i, len;
	char *p = buf;

	if (argc < 1) return -1;

	for (i=0; i<argc; i++) {
		if (p - buf > CTL_BUF_LEN) return -2;
		len = strlen(argv[i]);
		strncpy(p, argv[i], len);
		p[len] = ' ';
		p += len + 1;
	}
	*(--p) = '\0';

	return (int)(p - buf);
}



char *pad_str(char *buf, const char *str) {

	snprintf(buf, CTL_BUF_LEN, "%s\n", str);
	return buf + strlen(str) + 1;
}
