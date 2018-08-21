/*
 * SIP Manage -- by luojun
 * 
 */

#include "config.h"
#include "sip_man.h"
#include "cmedia.h"
#include "cmrtp.h"
#include "key_man.h"
#include "message.h"
#include "alsa_man.h"

pthread_t kernel_sip_pthread;
pthread_t kernel_sipcmd_pthread;
pthread_t kernel_key_pthread;
pthread_t kernel_cmedia_pthread;
pthread_t alsa_cmedia_pthread;



void Sig_handler (int sig){
	if (sig == SIGINT){
		printf (">>>>>>>>>>>>>>>>>>>press ctrl+c stop>>>>>>>>>>>>>>>>>>\n");
		Sip_released ();
		Uninit_Cmedia ();
		pthread_join (kernel_sip_pthread,NULL);
		exit (0);
	}
}

int main (int argc, char *argv[]){
	int check_ret = 0;
	char option[10];
	signal (SIGINT, Sig_handler);
	//Init_cmrtp();
	Init_Cmedia();
	/*********************************************************************************/
	check_ret = pipe(sip_msg.fd);
	if (check_ret != 0){
		printf ("creat pipe(sip_msg) is error !!\n");
		return 0;
	}
	check_ret = pipe(media_msg.fd);
	if (check_ret != 0){
		printf ("creat pipe(sip_msg) is error !!\n");
		return 0;
	}
	/*********************************************************************************/
	/*check_ret = pthread_create(&kernel_sip_pthread,NULL,Kernel_sip_pthread,NULL);
	if (check_ret != 0){
		printf ("pthread creat kernel_sip_pthread is error !!\n");
		return 0;
	}
	check_ret = pthread_create(&kernel_sipcmd_pthread,NULL,Kernel_sipcmd_pthread,NULL);
	if (check_ret != 0){
		printf ("pthread creat Kernel_sipcmd_pthread is error !!\n");
		return 0;
	}
	check_ret = pthread_create(&kernel_key_pthread,NULL,Kernel_key_pthread,NULL);
	if (check_ret != 0){
		printf ("pthread creat Kernel_sipcmd_pthread is error !!\n");
		return 0;
	}
	check_ret = pthread_create(&kernel_cmedia_pthread,NULL,Kernel_cmedia_pthread,NULL);
	if (check_ret != 0){
		printf ("pthread creat Kernel_sipcmd_pthread is error !!\n");
		return 0;
	}*/
	check_ret = pthread_create(&alsa_cmedia_pthread,NULL,Alsa_cmedia_pthread,NULL);
	if (check_ret != 0){
		printf ("pthread creat Kernel_sipcmd_pthread is error !!\n");
		return 0;
	}
	
	/*********************************************************************************/
	while (1){
		puts("Press 'h' to hangup all calls, 'q' to quit,'p' to call 4002");
		if (fgets(option, sizeof(option), stdin) == NULL) {
			puts("EOF while reading stdin, will quit now..");
			break;
		}
		if (option[0] == 'h'){
			Send_sip_message (sip_msg.fd[PIPE_WRITE],SIP_CALL_TERMINATE_CMD,NULL,NULL,0);
		}
		if (option[0] == 'p'){
			Send_sip_message (sip_msg.fd[PIPE_WRITE],SIP_CALL_PHONE_CMD,NULL,NULL,0);
		}
	}
}
