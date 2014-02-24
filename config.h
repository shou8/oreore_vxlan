#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED



struct config {
	char path[DEFAULT_BUFLEN];
	char param[DEFAULT_BUFLEN][DEFAULT_BUFLEN];
};



int get_config(char *config_path, char *message);



#endif /* CONFIG_H_INCLUDED */
