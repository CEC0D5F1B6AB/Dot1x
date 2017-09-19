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
	char buf[1024];//max 1k
	vsnprintf(buf, sizeof(buf), fmt, argp);
	Gb2312ToUtf8(buf, sizeof(buf));//the zte 802.1x using gb2312,so translate to utf-8
	fprintf(stdout, buf);
	syslog(LOG_INFO, buf);
	return;
}
