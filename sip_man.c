#include "config.h"
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
#include "sip_man.h"
#include "cmrtp.h"
#include "cmedia.h"
#include "event.h"
#include "message.h"

#define PROG_NAME "sipreg"
#define PROG_VER  "1.0"
#define UA_STRING "charmhope"
#define SYSLOG_FACILITY LOG_DAEMON
#define CALL_SESSION_MAX 1

static volatile int keepRunning = 1;
/*
int m_nCall_id = -1; //呼叫id
int m_nDlg_id = -1;  //绘话id
int m_nt_id = -1; */

typedef struct call_sav_str{
	int cid;
	int did;
	int in_use;
}call_sav_t;
call_sav_t call_sav_data;
int call_terminate_statue;

sip_msg_t sip_msg;

typedef struct regparam_t {
  int regid;
  int expiry;
  int auth;
} regparam_t;
struct regparam_t regparam = { 0, 3600, 0 };

struct eXosip_t *context_eXosip;
eXosip_event_t *event;

static int Init_call_sav (call_sav_t * call_sav_p){
	call_sav_p->cid = -1;
	call_sav_p->did = -1;
	call_sav_p->in_use = 0;
	return 1;
}

static int Add_call_sav (call_sav_t * call_sav_p,eXosip_event_t *event){
	int i;
	for (i=0;i<CALL_SESSION_MAX;i++){	
		if ((call_sav_p+i)->in_use == 1){
			continue;
		}
		else{
			(call_sav_p+i)->in_use = 1;
			(call_sav_p+i)->cid = event->cid;
			(call_sav_p+i)->did = event->did;
			printf ("Add one call sav!!\n");
			return i;
		}
	}
	return -1;
}

static int Remove_call_sav (call_sav_t * call_sav_p){
	int i;
	for (i=0;i<CALL_SESSION_MAX;i++){	
		if ((call_sav_p+i)->in_use == 0){
			continue;
		}
		else{
			(call_sav_p+i)->in_use = 0;
			(call_sav_p+i)->cid = -1;
			(call_sav_p+i)->did = -1;
			printf ("Remove one call sav!!\n");
			return i;
		}
	}
	return -1;
}

static int Check_call_sav (call_sav_t * call_sav_p){
	int i;
	for (i=0;i<CALL_SESSION_MAX;i++){	
		if ((call_sav_p+i)->in_use == 1){
			return i;
		}
	}
	return -1;
}

static void Invite_answer(struct eXosip_t *context_eXosip,eXosip_event_t *event,osip_message_t *answer){
    char tmp[4096]; 
    char localip[128];  

    if (-1 == event->tid)
    {
        return;
    }
        
    int i = eXosip_call_build_answer (context_eXosip,event->tid, 200, &answer);       
    if (i != 0){            
        printf ("This request msg is invalid!Cann't response!\n");           
        eXosip_call_send_answer (context_eXosip,event->tid, 400, NULL);         
    }      
    else {           
        eXosip_guess_localip (context_eXosip,AF_INET, localip, 128); // 获取本地IP
        //SDP格式
        snprintf (tmp, 4096,
            "v=0\r\n"
            "o=- 0 0 IN IP4 %s\r\n"// 用户名、ID、版本、网络类型、地址类型、IP地址 
            "s=%s\r\n" 
            "t=0 0\r\n"
            "m=audio %d RTP/AVP 0 8 101\r\n"    // 音频、传输端口、传输类型、格式列表 
			"c=IN IP4 %s\r\n"
			"b=TIAS:64000\r\n"
			"a=rtcp:%d IN IP4 %s\r\n"
			"a=sendrecv\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"          // 以下为具体描述格式列表中的  
            "a=rtpmap:8 PCMA/8000\r\n"  
            "a=rtpmap:101 telephone-event/8000\r\n",
            localip,UA_STRING,5000,localip,5001,localip);           
        osip_message_set_body (answer, tmp, strlen(tmp));            
        osip_message_set_content_type (answer, "application/sdp"); 
        eXosip_call_send_answer (context_eXosip,event->tid, 200, answer);           
        printf ("send 200 over!\n"); 
	}
} 

static void Call_answer (struct eXosip_t *context_eXosip,eXosip_event_t *event){
	sdp_message_t *remote_sdp;
	sdp_message_t *local_sdp;
	sdp_connection_t *conn = NULL;
	sdp_media_t *local_med = NULL;
	sdp_media_t *remote_med = NULL;

	remote_sdp = eXosip_get_remote_sdp(context_eXosip,event->did);
    local_sdp = eXosip_get_local_sdp(context_eXosip,event->did);
	if (remote_sdp == NULL){
		printf("No remote SDP body found for call\n");
	}
	if (remote_sdp != NULL && local_sdp != NULL){	

		printf("LOCAL SDP and REMOTE SDP found for call\n");
		conn = eXosip_get_audio_connection(remote_sdp);
		if (conn != NULL && conn->c_addr != NULL){
			printf ("REMOTE RTP ADDR is %s\n",conn->c_addr);
			//_snprintf(ca->remote_sdp_audio_ip, 50, conn->c_addr);
		}		
		remote_med = eXosip_get_audio_media(remote_sdp);
		if (remote_med != NULL && remote_med->m_port != NULL){
			printf ("REMOTE RTP PORT is %d\n",atoi(remote_med->m_port));
			//ca->remote_sdp_audio_port = atoi(remote_med->m_port);
		}
		local_med = eXosip_get_audio_media(local_sdp);
 		if (local_med != NULL && local_med->m_port != NULL){
			printf ("LOCAL RTP PORT is %d\n",atoi(local_med->m_port));
			//audio_port = atoi(local_med->m_port);
		}
	}
	Start_stream (conn->c_addr,atoi(local_med->m_port),atoi(remote_med->m_port));
	connect_wbhf_voipline (init_rtp_session_config.voip_line_id);	
	sdp_message_free(local_sdp);
	sdp_message_free(remote_sdp);
}

static void Call_ack(struct eXosip_t *context_eXosip,eXosip_event_t *event){
	sdp_message_t *remote_sdp;
	sdp_message_t *local_sdp;
	sdp_connection_t *conn = NULL;
	sdp_media_t *local_med = NULL;
	sdp_media_t *remote_med = NULL;

	remote_sdp = eXosip_get_remote_sdp(context_eXosip,event->did);
    local_sdp = eXosip_get_local_sdp(context_eXosip,event->did);
	if (remote_sdp == NULL){
		printf("No remote SDP body found for call\n");
	}
	if (remote_sdp != NULL && local_sdp != NULL){	
		printf("LOCAL SDP and REMOTE SDP found for call\n");
		conn = eXosip_get_audio_connection(remote_sdp);
		if (conn != NULL && conn->c_addr != NULL){
			printf ("REMOTE RTP ADDR is %s\n",conn->c_addr);
			//_snprintf(ca->remote_sdp_audio_ip, 50, conn->c_addr);
		}		
		remote_med = eXosip_get_audio_media(remote_sdp);
		if (remote_med != NULL && remote_med->m_port != NULL){
			printf ("REMOTE RTP PORT is %d\n",atoi(remote_med->m_port));
			//ca->remote_sdp_audio_port = atoi(remote_med->m_port);
		}
		local_med = eXosip_get_audio_media(local_sdp);
 		if (local_med != NULL && local_med->m_port != NULL){
			printf ("LOCAL RTP PORT is %d\n",atoi(local_med->m_port));
			//audio_port = atoi(local_med->m_port);
		}
	}
	Start_stream (conn->c_addr,atoi(local_med->m_port),atoi(remote_med->m_port));
	connect_wbhf_voipline (init_rtp_session_config.voip_line_id);	
	sdp_message_free(local_sdp);
	sdp_message_free(remote_sdp);
}

int Invite_call (void){
	int i;
	char localip[128];
	osip_message_t *invite = NULL;
	char tmp[4096];
	char source_call[100] = {0};
	char dest_call[100] = {0};
	sprintf(source_call, "sip:%s@%s", "102", "192.168.2.103");
	sprintf(dest_call, "sip:%s@%s", "4002", "192.168.2.103");
	i = eXosip_call_build_initial_invite (context_eXosip,&invite, dest_call, source_call, NULL, "This is a call for a conversation");
	if (i != 0){
        printf ("Intial INVITE failed!\n");
		return -1;
    }
	eXosip_guess_localip (context_eXosip,AF_INET, localip, 128);
	snprintf (tmp, 4096,
        "v=0\r\n"
        "o=- 0 0 IN IP4 %s\r\n"// 用户名、ID、版本、网络类型、地址类型、IP地址 
        "s=%s\r\n" 
        "t=0 0\r\n"
        "m=audio %d RTP/AVP 0 8 101\r\n"    // 音频、传输端口、传输类型、格式列表 
		"c=IN IP4 %s\r\n"
		"b=TIAS:64000\r\n"
		"a=rtcp:%d IN IP4 %s\r\n"
		"a=sendrecv\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"          // 以下为具体描述格式列表中的  
        "a=rtpmap:8 PCMA/8000\r\n"  
        "a=rtpmap:101 telephone-event/8000\r\n"
        "a=fmtp:101 0-11\r\n",
        localip,UA_STRING,5000,localip,5001,localip); 
	osip_message_set_body (invite, tmp, strlen(tmp));
    osip_message_set_content_type (invite, "application/sdp");
	eXosip_lock (context_eXosip);
    i = eXosip_call_send_initial_invite (context_eXosip,invite);
	if (i != 0){
        printf ("Send INVITE failed!\n");
    }
    eXosip_unlock (context_eXosip);
	return 1;
}

static int Init_sip_data (void){
	char transport[5] = {0};
	char proxy[20] = {0};
	char contact[50] = {0};
  	char fromuser[20] = {0};
	char username[10] = {0};
	char password[10] = {0};
	int check_ret = 0;
	int port = 5060;
	osip_message_t *reg = NULL;
	/*************************ON_DEBUG*********************/
	TRACE_ENABLE_LEVEL(6);
	TRACE_INITIALIZE(6,stderr);
	Init_call_sav (&call_sav_data);
	/*************************LOAD_INIT*********************/
	sprintf (proxy,"sip:192.168.2.103");
	sprintf (fromuser,"sip:102@192.168.2.105");
	sprintf (contact,"sip:102@192.168.2.105:5060");
	sprintf (username,"102");
	sprintf (password,"102");
	/*************************SET_IP_TRANSPORT*********************/
	snprintf (transport, sizeof (transport), "%s", "UDP");
	if (osip_strcasecmp (transport, "UDP") != 0 && osip_strcasecmp (transport, "TCP") != 0 && osip_strcasecmp (transport, "TLS") != 0 && osip_strcasecmp (transport, "DTLS") != 0) {
		printf("wrong transport parameter\n");
		return (-1);
  	}
	context_eXosip = eXosip_malloc ();
	if (eXosip_init (context_eXosip)) {
		printf("eXosip_init failed\n");
		return (1);
	}
	check_ret = -1;
	if (osip_strcasecmp (transport, "UDP") == 0) {
		check_ret = eXosip_listen_addr (context_eXosip, IPPROTO_UDP, NULL, port, AF_INET, 0);
	}
	else if (osip_strcasecmp (transport, "TCP") == 0) {
		check_ret = eXosip_listen_addr (context_eXosip, IPPROTO_TCP, NULL, port, AF_INET, 0);
	}
	else if (osip_strcasecmp (transport, "TLS") == 0) {
		check_ret = eXosip_listen_addr (context_eXosip, IPPROTO_TCP, NULL, port, AF_INET, 1);
	}
	else if (osip_strcasecmp (transport, "DTLS") == 0) {
		check_ret = eXosip_listen_addr (context_eXosip, IPPROTO_UDP, NULL, port, AF_INET, 1);
	}
	if (check_ret) {
		printf("eXosip_listen_addr failed\n");
		return (-1);
	}
	/*****************************SET SIP CONFIGURE****************************/
	eXosip_set_user_agent (context_eXosip, UA_STRING);
	printf("username: %s",username);
	printf("password: %s\n",password);
	if (eXosip_add_authentication_info (context_eXosip, username, username, password, NULL, NULL)) {
      printf("eXosip_add_authentication_info failed\n");
      return(-1);
    }
	/*****************************SET SIP REGEISTER****************************/
	regparam.regid = eXosip_register_build_initial_register (context_eXosip, fromuser, proxy, contact, regparam.expiry, &reg);
	if (regparam.regid < 1) {
		 printf("eXosip_register_build_initial_register failed\n");
		 return(-1);
	}
	check_ret = eXosip_register_send_register (context_eXosip, regparam.regid, reg);
	if (check_ret != 0) {
		printf("eXosip_register_send_register failed\n");
		return(-1);
	}
	else{
		return 1;
	}
}



void Sip_released (void){
	keepRunning = 0;
}

static void Terminate_call (void){
	eXosip_lock (context_eXosip);
	call_terminate_statue = 1;
	eXosip_call_terminate (context_eXosip,call_sav_data.cid, call_sav_data.did);
	eXosip_unlock (context_eXosip);
}

static void Sip_cmd_mod (int fd, short event, void* arg){
	char cmd_msg[MESSAGE_MAX_LEN] = {0};
	message_t message_sip;
	if (event & EV_READ){
		read (fd,cmd_msg,sizeof(cmd_msg));
		if (Recive_sip_message (cmd_msg,&message_sip) == 1){
			switch (message_sip.cmd){
				case SIP_CALL_PHONE_CMD:{
					printf ("Rev call phone cmd !!\n");
					Invite_call ();
				}break;
				case SIP_CALL_TERMINATE_CMD:{
					printf ("Rev call terminate cmd !!\n");
					Terminate_call ();
				}break;
				default:{
				
				}break;
			}
		}
	}
}

void *Kernel_sipcmd_pthread (void *arg){
	struct event* sipcmd_ev;
	struct event_base* sip_base;
	sip_base = event_base_new();
	while (1){
		sipcmd_ev = event_new(sip_base,sip_msg.fd[PIPE_READ],EV_READ|EV_PERSIST,Sip_cmd_mod,NULL);
		event_add(sipcmd_ev, NULL);
		event_base_dispatch(sip_base);
		sleep (1);	
	}
	return(void *)0;
}


void *Kernel_sip_pthread(void *arg){
	if (Init_sip_data () == -1){
		exit (1);
	}
	else{
		printf (">>>>>>>>>>>>>>>>>SIP MANAGE IS RUNNING>>>>>>>>>>>>>>>>>>>>>>\n");
	}
	for (;keepRunning;) {
		static int counter = 0;
		osip_message_t *answer = NULL; 
		counter++;
		if (counter % 60000 == 0) {
      		struct eXosip_stats stats;
			memset (&stats, 0, sizeof (struct eXosip_stats));
			eXosip_lock (context_eXosip);
			eXosip_set_option (context_eXosip, EXOSIP_OPT_GET_STATISTICS, &stats);
			eXosip_unlock (context_eXosip);
			printf("eXosip stats: inmemory=(tr:%i//reg:%i) average=(tr:%f//reg:%f)\n", stats.allocated_transactions, stats.allocated_registrations, stats.average_transactions, stats.average_registrations);
    	}
		if (!(event = eXosip_event_wait (context_eXosip, 0, 1))) {
			eXosip_lock (context_eXosip);
			eXosip_automatic_action (context_eXosip);
      		osip_usleep (10000);
			eXosip_unlock (context_eXosip);
      		continue;
		}
		eXosip_lock (context_eXosip);
    	eXosip_automatic_action (context_eXosip);
		
		printf ("event_type code is %d\n",event->type);
		switch (event->type) {
			case EXOSIP_MESSAGE_NEW:{//新的消息到来       
            	printf ("EXOSIP_MESSAGE_NEW!\n");  
            	if (MSG_IS_MESSAGE (event->request)){//如果接受到的消息类型是MESSAGE
                	osip_body_t *body;          
                	osip_message_get_body (event->request, 0, &body);          
                	printf ("I get the msg is: %s\n", body->body);          
                	//printf ("the cid is %s, did is %s\n", je->did, je->cid);       '
				}//按照规则，需要回复200 OK信息        
            	eXosip_message_build_answer (context_eXosip,event->tid, 200,&answer);       
            	eXosip_message_send_answer (context_eXosip,event->tid, 200,answer);   
				osip_message_free (answer);
			}
            break;  
    		case EXOSIP_REGISTRATION_SUCCESS:{
      			printf("registrered successfully\n");
			}break;
			case EXOSIP_REGISTRATION_FAILURE:{// 注册失败
            	printf("EXOSIP_REGISTRATION_FAILURE\n");
			}
            break;
			case EXOSIP_CALL_INVITE:{
				printf ("call_id is %d, dialog_id is %d \n", event->cid, event->did);  
				if (Add_call_sav(&call_sav_data,event) == -1){
					printf ("Add call sav is over max seesion!!\n");
					eXosip_call_send_answer (context_eXosip,event->tid,400,NULL);
				}
				else{
            		eXosip_call_send_answer (context_eXosip,event->tid, 180, NULL);  
					Invite_answer(context_eXosip,event,answer);
				}
			}break;
			case EXOSIP_CALL_ANSWERED:{
				printf ("ok! connected!\n"); 
				if (Add_call_sav(&call_sav_data,event) == -1){
					printf ("Add call sav is over max seesion!!\n");
					/***************UNKNOW ANSWER************************************/
				}
				else{
					Call_answer (context_eXosip,event);
					printf ("call_id is %d, dialog_id is %d \n", event->cid, event->did); 
				}
			}break;
			case EXOSIP_CALL_CLOSED:{
				if (Remove_call_sav(&call_sav_data) == -1){
					printf ("Remove call is over low seesion!!\n");
				}
				else{
					Stop_stream ();
					disconnect_wbhf_voipline (init_rtp_session_config.voip_line_id);
				}
				printf ("call_id is %d, dialog_id is %d \n", event->cid, event->did); 
            	printf ("the other sid closed!\n");	
			}break;
			case EXOSIP_CALL_ACK:{
            	printf ("ACK received!\n");
				Call_ack (context_eXosip,event);
            }break;
			case EXOSIP_CALL_MESSAGE_NEW:{
				printf ("EXOSIP_CALL_MESSAGE_NEW!\n");
			}break;
			case EXOSIP_CALL_RELEASED:{
				if (call_terminate_statue == 1){
					call_terminate_statue = 0;
					if (Check_call_sav (&call_sav_data)!= -1){
						printf ("Remove one call session!!\n");
						Remove_call_sav(&call_sav_data);
						Stop_stream ();
						disconnect_wbhf_voipline (init_rtp_session_config.voip_line_id);
					}
				}
            	printf ("CALL_RELEASED %s\n",event->textinfo);
            }break;
			default:{
			}break;
		}
		eXosip_unlock (context_eXosip);
    	eXosip_event_free (event);
	}
	eXosip_quit (context_eXosip);
  	osip_free (context_eXosip);
	printf (">>>>>>>>>>>>>>>>>SIP MANAGE IS CLOSED>>>>>>>>>>>>>>>>>>>>>>\n");
	return(void *)0; 
}
