#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"



struct config {
	char param[DEFAULT_BUFLEN];
	char values[DEFAULT_BUFLEN][DEFAULT_BUFLEN];
};



char param[][DEFAULT_BUFLEN] = {
	"multicast_address",
	"multicast_interface",
	"port",
	"unix_socket",
	"vxlan_instance"
};



static int match_char(char c, char *str, int len);
static int rm_extra_right_char(char *str, char *rm_chars);
static char *rm_extra_left_char(char *str, char *rm_chars);
static char *rm_extra_char(char *str);


int get_config(char *config_path, char *message) {

	FILE *fp;

	if ((fp = fopen(config_path, "r")) == NULL)
		return -1;

	char bracket;
	char *p, *par_head, *val_head, line[DEFAULT_BUFLEN];
	int param_size = sizeof(param) / sizeof(param[0]);
	int line_num = 1;

	while (fgets(line, DEFAULT_BUFLEN, fp) != NULL) {

		// if this line includes '#', the point is regarded as end of line
		bracket = 0;
		for (p=line; *p != '\0'; p++) {
			if ( *p == '"' || *p == '\'' ) {
				bracket ^= *p;
				break;
			} else if ( *p == '#' && bracket == 0 ) {
				*p = '\0';
				break;
			}
		}

		// Remove right extra space and tabs
		if (rm_extra_right_char(line, " \t") == 0)
			continue;

		// Delete left extra space and tabs
		par_head = rm_extra_left_char(line, " \t");

		// Search '=' separator
		val_head = strchr(par_head, '=');
		if (val_head == NULL) {
//			snprintf();
			return -2;
		}

		*val_head = '\0';
		val_head++;

		printf("param:'%s', value:'%s'\n", rm_extra_char(par_head), rm_extra_char(val_head));

//		for (i=0; i<param_size; i++)
//			if (str_cmp(param[i], line)) break;
		line_num++;
	}

	return 0;
}



static int match_char(char c, char *str, int len) {
	
	int i;
	for (i = 0; i < len; i++)
		if (c == str[i]) return 1;

	return 0;
}



static int rm_extra_right_char(char *str, char *rm_chars) {

	int i = strlen(str) - 1;
	if (str[i] == '\n') i--;

	int rm_chars_len = strlen(rm_chars);
	for ( ; i >= 0; i--)
		if (! match_char(str[i], rm_chars, rm_chars_len)) break;

	str[++i] = '\0';
	return i;
}



static char *rm_extra_left_char(char *str, char *rm_chars) {

	char *p;
	int rm_chars_len = strlen(rm_chars);
	for (p=str; *p != '\0'; p++)
		if (! match_char(*p, rm_chars, rm_chars_len)) break;
	return p++;
}



static char *rm_extra_char(char *str) {

	rm_extra_right_char(str, " \t");
	str = rm_extra_left_char(str, " \t");

	// for bracket
	int len = strlen(str);
	if (str[0] == '"' || str[0] == '\'') {
		if (str[len-1] != str[0]) return NULL; // Backet is inconsistency
		str[len-1] = '\0';
		str++;
	}

	return str;
}
