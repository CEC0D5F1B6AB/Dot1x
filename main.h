#include "md5.h"
#include "rc4.h"
#include "cp936.h"
#include <syslog.h>

#define TIMEOUT 1

char *username, *password, *interface;

void do_log(char * fmt, ...){
	va_list argp;
	va_start(argp, fmt);
	va_end(argp);
	vfprintf(stdout, fmt, argp);
	vsyslog(LOG_INFO, fmt, argp);
	return;
}