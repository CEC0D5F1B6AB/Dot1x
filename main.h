#include "md5.h"
#include "rc4.h"

#define TIMEOUT 3

char *username, *password, *interface;

void writelog(int priority, char * fmt, ...){
	va_list argp;
	va_start(argp, fmt);
	va_end(argp);
	vfprintf(stdout, fmt, argp);
	syslog(priority, fmt, argp);
	return;
}