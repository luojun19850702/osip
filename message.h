#ifndef _MESSAGE_H
#define _MESSAGE_H

#define MESSAGE_HEAD	0x78
#define MESSAGE_END		0x72

#define MESSAGE_MAX_LEN	250

extern pthread_mutex_t message_sip_lock;

typedef struct message_str{
	unsigned char cmd;
	char data[MESSAGE_MAX_LEN];
	char len;
}message_t;

extern void Init_mutex_message (void);

extern void Print_xdata_cs (char * print_data,int len);
extern void Send_sip_message (int file_device,char cmd,char * s_p,sem_t * sem_p,int len);
extern int Recive_sip_message (char * r_p,message_t * m_p);
extern void Send_media_message (int file_device,char cmd,char * s_p,sem_t * sem_p,int len);
extern int Recive_media_message (char * r_p,message_t * m_p);
#endif
