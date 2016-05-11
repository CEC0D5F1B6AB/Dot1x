////////////////////////////////////////////////////////////////////
//							_ooOoo_								  //
//						   o8888888o							  //	
//						   88" . "88							  //	
//						   (| ^_^ |)							  //	
//						   O\  =  /O							  //
//						____/`---'\____							  //						
//					  .'  \\|     |//  `.						  //
//					 /  \\|||  :  |||//  \						  //	
//				    /  _||||| -:- |||||-  \						  //
//				    |   | \\\  -  /// |   |						  //
//					| \_|  ''\---/''  |   |						  //		
//					\  .-\__  `-`  ___/-. /						  //		
//				  ___`. .'  /--.--\  `. . ___					  //	
//				."" '<  `.___\_<|>_/___.'  >'"".				  //
//			  | | :  `- \`.;`\ _ /`;.`/ - ` : | |				  //	
//			  \  \ `-.   \_ __\ /__ _/   .-` /  /                 //
//		========`-.____`-.___\_____/___.-`____.-'========		  //	
//				             `=---='                              //
//		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        //
//         佛祖保佑       永无BUG		永不修改				  //
////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/if_packet.h>

#include "main.h"
#include "eap.c"

int usage(const char *progname)
{
	fprintf(stderr, "Dot1x 1.1 By Tuber\n\n"
			"Usage: %s <options>\n"
			"\n"
			"Options:\n"
			"\t-u <username>\n"
			"\t-p <password>\n"
			"\t-i <ifname>\n"
			"\t-b <background>\n"
			"\n",
		progname);
	return(-1);
}

int main(int argc, char *argv[]){
	int ch, background = 0;
	setbuf(stdout,NULL);
	
	openlog("Dot1x", 0, LOG_AUTH); 

	if(getuid() != 0){
        fprintf(stderr, "Check Need Root Power.\n");
        exit(-1);
    }
    
	while ((ch = getopt(argc, argv, "bi:u:p:")) != -1) {
		switch(ch) {
			case 'u':
				username = optarg;
				break;
			case 'p':
				password = optarg;
				break;
			case 'i':
				interface = optarg;
				break;
			case 'b':
				background = 1;
				break;
			default:
				usage(argv[0]);
				return(-1);
		}
	}
	
	if(username == NULL || password == NULL || interface == NULL){
		usage(argv[0]);
		return(-1);
	}
	
	if(strlen(interface) > IFNAMSIZ){
		fprintf(stderr, "Error interface.\n");
	}
	
	pid_t fpid = 0;
	if(background){
		fpid = fork();
	}
	
	if (fpid < 0){
		writelog(LOG_ERR, "fork: %s\n", strerror(errno));
		return(-1);
	}else if(fpid == 0){
		eap_auth();
	}
	
	closelog();
	return(1);
}