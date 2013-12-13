#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>



void split_32to8(uint32_t val, uint8_t *arr) {

	arr[0] = (uint8_t)(val);
	arr[1] = (uint8_t)(val >> 8);
	arr[2] = (uint8_t)(val >> 16);
}



int str2uint8arr(char *str, uint8_t *arr) {

	uint32_t val = strtol(str, NULL, 0);
	if (errno != ERANGE) return -1;
	split_32to8(val, arr);
	return 0;
}



