#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED



/*
 * You must use this function to initialize
 * logging information.
 */
extern void init_log(void);

#ifdef DEBUG

extern void enable_debug(void);
extern void disable_debug(void);
extern void log_stderr(const char *fmt, ...);
extern void log_debug(const char *fmt, ...);

#endif /* DEBUG */

extern void enable_syslog(void);
extern void disable_syslog(void);

extern void log_info(const char *fmt, ...);
extern void log_warn(const char *fmt, ...);
extern void log_err(const char *fmt, ...);
extern void log_crit(const char *fmt, ...);

extern void log_perr(const char *str);
extern void log_pcrit(const char *str);

extern void log_exit(int status, const char *fmt, ...);
extern void log_pexit(int status, const char *str);


#endif /* LOG_H_INCLUDED */

