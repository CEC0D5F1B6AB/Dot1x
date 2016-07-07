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
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <syslog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/if_packet.h>

#include "eap.h"

int send_eth(unsigned short int proto, unsigned short int len){
	int t = sizeof(struct eth) + len;
	memcpy(eth->dest, des_addr, 6);
	memcpy(eth->source, src_addr, 6);
	eth->proto = htons(proto);
	t = send(sockfd, buf, t, 0);
	if(t < 0){
		do_log("Send: %s\n", strerror(errno));
		sockfd = eap_init();
		status = EAP_FAILURE;
		do_log("Reinit the socket\n");
	}
	return(t);
}

int send_eapol(unsigned char type, unsigned short int len){
	int t = sizeof(struct eapol) + len;
	eapol->ver = 1;
	eapol->type = type;
	eapol->len = htons(len);
	t = send_eth(ETH_P_PAE, t);
	return(t);
}

int send_eap(unsigned char code, unsigned short int len){
	int t = sizeof(struct eap) + len;
	eap->code = code;
	eap->len = htons(t);
	t = send_eapol(EAPOL_EAPPACKET, t);
	return(t);
}

int eapol_start(){
	int t;
	t = send_eapol(EAPOL_START, 0);
	do_log("EAPOL Start\n");
	return(t);
}

int eapol_logoff(){
	int t;
	t = send_eapol(EAPOL_LOGOFF, 0);
	do_log("EAPOL Logoff\n");
	return(t);
}

int eap_identity(){
	int t;
	do_log("EAP Request Identity\n");
	t = strlen(strcpy(last, username));
	t = send_eap(EAP_RESPONSE, t);
	do_log("EAP Response Identity\n");
	return(t);
}

int eap_md5(){
	int t;
	unsigned char tb[PKT_SIZE];
	MD5_CTX context;
	do_log("EAP Request MD5\n");
	t = 0;
	tb[t++] = eap->id;
	t += strlen(strcat(strcpy(tb + t, password), salt));
	memcpy(tb + t, md5->value, 16);
	t += 16;
	MD5Init(&context);
	MD5Update(&context, tb, t);
	MD5Final(tb + t, &context);
	memcpy(md5->value, tb + t, 16);
	t = sizeof(struct md5) + strlen(strcpy(md5->username, username)) - 1;
	t = send_eap(EAP_RESPONSE, t);
	do_log("EAP Response MD5\n");
	return(t);
}

int eapol_key_rc4(){
	int t;
	unsigned char enckey[]={0x02,0x0E,0x05,0x04,0x66,0x40,0x19,0x75,0x06,0x06,0x00,0x16,0xD3,0xF3,0xAC,0x02};
	unsigned char wholekey[20];
	do_log("EAPOL Request Key RC4\n");
	t = sizeof(struct key) + ntohs(key->keylen) - 16;
	//key
	memcpy(wholekey, key->keyiv, 16);
	memcpy(wholekey + 16, key->rc + 4, 4);
	rc4_crypt(enckey, ntohs(key->keylen), wholekey, 20);
	memcpy(key->key, enckey, ntohs(key->keylen));
	//hash
	bzero(key->keysignature, 16);
	hmac_md5((unsigned char *)eapol, sizeof(struct eapol) + t, &key->keyindex, 1, wholekey);
	memcpy(key->keysignature, wholekey, 16);
	t = send_eapol(EAPOL_KEY, t);
	do_log("EAPOL Response Key RC4\n");
	return(t);
}

void e_stop(int signo){
	e = 0;
}

int eap_init(){
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_PAE));
	if (sockfd < 0){
		do_log("Socket: %s\n", strerror(errno));
		return(-1);
	}
	//socket
	struct ifreq ifr;
	bzero(&ifr, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, interface);
	if(ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0){
		do_log("Ioctl: %s\n", strerror(errno));
		return(-1);
	}
	struct sockaddr_ll addr;
	bzero(&addr, sizeof(struct sockaddr_ll));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_ll)) < 0){
		do_log("Bind: %s\n", strerror(errno));
		return(-1);
	}
	//bind
	struct timeval timeout={TIMEOUT,0};
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) !=0){
		do_log("Setsockopt: %s\n", strerror(errno));
		return(-1);
	}
	//timeout
	int size = sizeof(struct sockaddr_ll);
	if (getsockname(sockfd, (struct sockaddr *)&addr, &size) < 0){
		do_log("Getsockname: %s\n", strerror(errno));
		return(-1);
	}
	memcpy(src_addr, addr.sll_addr, addr.sll_halen);
	//mac
	signal(SIGINT, e_stop); 
	signal(SIGQUIT, e_stop);
	signal(SIGTERM, e_stop);
	signal(SIGSTOP, e_stop);
	//signal
	return(sockfd);
}

int get_netlink_status(){
	struct ifreq ifr;
	struct ethtool_value edata;
	edata.cmd = ETHTOOL_GLINK;
	edata.data = 0;
	bzero(&ifr, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, interface);
	ifr.ifr_data = (char *)&edata;
	if(ioctl(sockfd, SIOCETHTOOL, &ifr) < 0){
		do_log("Ioctl: %s\n", strerror(errno));
		return(0);
	}
	return(edata.data);
}

int eap_auth(){
	sockfd = eap_init();
	if (sockfd < 0){
		return(-1);
	}
	
	e = 1;
	int t, tag;
	pid_t fpid = 0;
	
	count = count_aim = 0;
	
	while(e > 0){
		t = recv(sockfd, buf, sizeof(buf), 0);
		if(status == EAP_FAILURE){
			if(count > 0){
				if(count == 1){
					do_log("Sleep %d second to retry\n", count_aim);
				}
				if(count++ < count_aim){
					count_aim += 10;
					if(count_aim > 300){
						count_aim = 300;
					}
					continue;
				}else{
					count = 0;
				}
			}
		}
		if(t > 0){
			if(eth->proto == htons(ETH_P_PAE) && !memcmp(eth->dest, src_addr, 6)){
				tag = 1;
				switch(eapol->type){
					case EAPOL_EAPPACKET:
						switch(eap->code){
							case EAP_REQUEST:
								status = EAP_REQUEST;
								switch(eap->type){
									case EAP_TYPE_IDENTITY:
										eap_identity();
										break;
									case EAP_TYPE_NOTIFICATION:
										do_log("EAP Request Notification\n%s\n", last);
										break;
									case EAP_TYPE_MD5:
										eap_md5();
										break;
									default:
										do_log("Unknow eap type: %d\n", eap->type);
										break;
								}
								break;
							case EAP_SUCCESS:
								status = EAP_SUCCESS;
								count = count_aim = 0;
								do_log("EAP Success\n");
								break;
							case EAP_FAILURE:
								status = EAP_FAILURE;
								count++;
								last[last[0] + 1] = '\0';
								do_log("EAP Failure\n");
								do_log("%s\n", last + 1);
								break;
							default:
								do_log("Unknow eapol type: %d\n", eap->code);
								break;
						}
						break;
					case EAPOL_KEY:
						switch(eap->code){
							case EAP_KEY_RC4:
								eapol_key_rc4();
								break;
							default:
								do_log("Unknow key type: %d\n", eap->code);
								break;
						}
						break;
					default:
						do_log("Unknow packet type: %d\n", eapol->type);
						break;
				}
			}
		}else{
			t = get_netlink_status();
			if(t == 1 && errno == EAGAIN){
				if(status != EAP_SUCCESS){
					if(status == EAP_FAILURE){
						do_log("Timeout,try to reconnection\n");
					}
					eapol_start();
				}
			}else if(t == 0){
				status = EAP_FAILURE;
				if(tag > 0){
					do_log("Waiting for link...\n");
					tag = 0;
				}	
			}
		}
	}
	//end
	if(status == EAP_SUCCESS){
		eapol_logoff();
	}
	close(sockfd);
	return 0;
}