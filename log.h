#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED



/*
 * You must use this function to initialize
 * logging information.
 */
void init_log(void);

#ifdef DEBUG

void enable_debug(void);
void disable_debug(void);
void log_stderr(const char *fmt, ...);
void log_debug(const char *fmt, ...);

#endif /* DEBUG */

void enable_syslog(void);
void disable_syslog(void);

void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_err(const char *fmt, ...);
void log_crit(const char *fmt, ...);

void log_perr(const char *str);
void log_pcrit(const char *str);

void log_iexit(const char *fmt, ...);
void log_cexit(const char *fmt, ...);
void log_pcexit(const char *str);


#endif /* LOG_H_INCLUDED */

