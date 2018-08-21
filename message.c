#include "config.h"
#include "message.h"

pthread_mutex_t message_sip_se_lock;
pthread_mutex_t message_sip_re_lock;
pthread_mutex_t message_media_se_lock;
pthread_mutex_t message_media_re_lock;

void Init_mutex_message (void){
	
}

void Send_sip_message (int file_device,char cmd,char * s_p,sem_t * sem_p,int len){
	int j = 0;
	int i = 0;
	char send_message[100] = {0};
	pthread_mutex_lock(&message_sip_se_lock);
	char * send_p = send_message;
	*(send_p+i++) = MESSAGE_HEAD;
	*(send_p+i++) = len;
	*(send_p+i++) = cmd;
	for (j=0;j<len;j++){
		*(send_p+i++) = *(s_p+j);
	}
	*(send_p+i++) = MESSAGE_END;
	write (file_device,send_p,len+4);
	//sem_wait(sem_p);
	pthread_mutex_unlock(&message_sip_se_lock);
}

void Send_media_message (int file_device,char cmd,char * s_p,sem_t * sem_p,int len){
	int j = 0;
	int i = 0;
	char send_message[100] = {0};
	pthread_mutex_lock(&message_media_se_lock);
	char * send_p = send_message;
	*(send_p+i++) = MESSAGE_HEAD;
	*(send_p+i++) = len;
	*(send_p+i++) = cmd;
	for (j=0;j<len;j++){
		*(send_p+i++) = *(s_p+j);
	}
	*(send_p+i++) = MESSAGE_END;
	write (file_device,send_p,len+4);
	//sem_wait(sem_p);
	pthread_mutex_unlock(&message_media_se_lock);
}

int Recive_sip_message (char * r_p,message_t * m_p){
	char len = 0;
	pthread_mutex_lock(&message_sip_re_lock);
	if (*(r_p+0) != MESSAGE_HEAD){
		return 0;
	}
	len = *(r_p+1);
	if (*(r_p+len+3) != MESSAGE_END){
		return 0;
	}
	m_p->cmd = *(r_p+2);
	memcpy (&m_p->data[0],(r_p+3),len);
	m_p->len = len;
	pthread_mutex_unlock(&message_sip_re_lock);
	return 1;
}

int Recive_media_message (char * r_p,message_t * m_p){
	char len = 0;
	pthread_mutex_lock(&message_media_re_lock);
	if (*(r_p+0) != MESSAGE_HEAD){
		return 0;
	}
	len = *(r_p+1);
	if (*(r_p+len+3) != MESSAGE_END){
		return 0;
	}
	m_p->cmd = *(r_p+2);
	memcpy (&m_p->data[0],(r_p+3),len);
	m_p->len = len;
	pthread_mutex_unlock(&message_media_re_lock);
	return 1;
}

void Print_xdata_cs (char * print_data,int len){
	int i = 0;
	printf ("print data is ");
	for (i=0;i<len;i++){
		printf ("%x ",*(print_data+i));
	}
	printf ("end\n");
}



