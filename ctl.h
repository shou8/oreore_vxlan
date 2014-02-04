#ifndef CTL_H_INCLUDED
#define CTL_H_INCLUDED



struct cmd_entry {
	const char *name;
	int (*exec)(char *buf, int cmd_i, int argc, char *argv[]);
	const char *arg;
	const char *comment;
};



extern struct cmd_entry cmd_t[];
extern int cmd_len;



void ctl_loop(char *dom);
void *outer_loop_thread(void *args);



#endif /* CTL_H_INCLUDED */
