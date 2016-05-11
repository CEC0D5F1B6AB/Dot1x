#include "md5.h"
#include "rc4.h"
#include "cp936.h"

#define TIMEOUT 5

char *username, *password, *interface;

void writelog(int priority, char * fmt, ...){
	va_list argp;
	va_start(argp, fmt);
	va_end(argp);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, argp);
	Gb2312ToUtf8(buf, sizeof(buf));
	fprintf(stdout, buf);
	syslog(priority, buf);
	return;
}