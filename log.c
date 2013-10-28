#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <errno.h>

#include "base.h"
#include "log.h"



#define LOG_LINELEN		256

#define DEBUG_DISABLE	0x00	// Include DEBUG facility (default)
#define DEBUG_ENABLE	0x01	// Exclude DEBUG facility

#define SYSLOG_ENABLE	0x00	// Log on syslog (default)
#define SYSLOG_DISABLE	0x01	// Log on stderr



// Logging on syslog or stderr
static int _syslog_mode = SYSLOG_ENABLE;

#ifdef DEBUG
// Using for message information
static int _pid;						// Process ID
static char _h_name[HOST_NAME_MAX];		// Host Name

static int _debug_mode = DEBUG_ENABLE;
#endif /* DEBUG */



static void _print_log(int level, const char *fmt, ...);
static void _print_log_v(int level, const char *fmt, va_list ap);



void enable_syslog(void);
void log_crit(const char *fmt, ...);
void log_pcirt(const char *str);



/*
 * Chage log mode
 *
 */
#ifdef DEBUG

void enable_debug(void) {

	_debug_mode = DEBUG_ENABLE;
}



void disable_debug(void) {

	_debug_mode = DEBUG_DISABLE;
}

#endif /* DEBUG */



/*
 * Get logging information
 */
void init_log(void) {

	enable_syslog();

#ifdef DEBUG

	_pid = getpid();

	if (gethostname(_h_name, sizeof(_h_name)) != 0)
		log_pcexit(NULL);

	disable_syslog();

#endif /* DEBUG */
}



/*
 * After "enable_syslog" function is used,
 * All messages is only logged on syslog.
 *
 * After "disable_syslog" function is used,
 * All messages is only logged on stderr.
 */
void enable_syslog(void) {

	_syslog_mode = SYSLOG_ENABLE;
}



void disable_syslog(void) {

	_syslog_mode = SYSLOG_DISABLE;
}



#ifdef DEBUG

void log_stderr(const char *fmt, ...) {

	va_list ap;
	int mode = _syslog_mode;
	
	va_start(ap, fmt);
	disable_syslog();
	_print_log_v(LOG_DEBUG, fmt, ap);
	if (mode == SYSLOG_ENABLE)
		enable_syslog();
	va_end(ap);
}



void log_debug(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	if (_debug_mode == DEBUG_ENABLE)
		_print_log_v(LOG_DEBUG, fmt, ap);
	va_end(ap);
}

#endif /* DEBUG */



void log_info(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	_print_log_v(LOG_INFO, fmt, ap);
	va_end(ap);
}



void log_warn(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	_print_log_v(LOG_WARNING, fmt, ap);
	va_end(ap);
}



void log_err(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	_print_log_v(LOG_ERR, fmt, ap);
	va_end(ap);
}



void log_crit(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	_print_log_v(LOG_CRIT, fmt, ap);
	va_end(ap);
}



void log_perr(const char *str) {

	if(str == NULL)
		_print_log(LOG_ERR, "%s\n", strerror(errno));
	else
		_print_log(LOG_ERR, "%s: %s\n", str, strerror(errno));
}



void log_pcrit(const char *str) {

	if(str == NULL)
		_print_log(LOG_CRIT, "%s\n", strerror(errno));
	else
		_print_log(LOG_CRIT, "%s: %s\n", str, strerror(errno));
}



void log_iexit(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	_print_log_v(LOG_INFO, fmt, ap);
	va_end(ap);
	exit(EXIT_SUCCESS);
}



void log_cexit(const char *fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	_print_log_v(LOG_CRIT, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}



void log_pcexit(const char *str) {

	if(str == NULL)
		_print_log(LOG_CRIT, "%s\n", strerror(errno));
	else
		_print_log(LOG_CRIT, "%s: %s\n", str, strerror(errno));

	exit(EXIT_FAILURE);
}




static void _print_log(int level, const char *fmt, ...) {

	char line[LOG_LINELEN];
	va_list ap;

	va_start(ap, fmt);

#ifndef DEBUG
	if (_syslog_mode == SYSLOG_ENABLE) {
		openlog(DAEMON_NAME, LOG_CONS | LOG_PID, level);
		vsnprintf(line, LOG_LINELEN, fmt, ap);
		syslog(level, line);
		closelog();
	} else {
#endif /* DEBUG */
		time_t t;

		time(&t);
		strncpy(line, ctime(&t), sizeof(line));

		int len = strlen(line);
		line[len-1] = '\0';
		fprintf(stderr, "%s %s "DAEMON_NAME"[%d]: ", line, _h_name, _pid);
		vfprintf(stderr, fmt, ap);
#ifndef DEBUG
	}
#endif /* DEBUG */

	va_end(ap);
}



static void _print_log_v(int level, const char *fmt, va_list ap) {

	char line[LOG_LINELEN];

#ifndef DEBUG
	if (_syslog_mode == SYSLOG_ENABLE) {
		openlog(DAEMON_NAME, LOG_CONS | LOG_PID, level);
		vsnprintf(line, LOG_LINELEN, fmt, ap);
		syslog(level, line);
		close(log);
	} else {
#endif /* DEBUG */
		time_t t;

		time(&t);
		strncpy(line, ctime(&t), sizeof(line));

		int len = strlen(line);
		line[len-1] = '\0';
		fprintf(stderr, "%s %s "DAEMON_NAME"[%d]: ", line, _h_name, _pid);
		vfprintf(stderr, fmt, ap);
#ifndef DEBUG
	}
#endif /* DEBUG */

}



