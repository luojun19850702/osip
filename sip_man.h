#ifndef _SIP_MAN_H
#define _SIP_MAN_H

#define SIP_CALL_PHONE_CMD		0
#define SIP_CALL_TERMINATE_CMD	1

typedef struct sip_msg_str{
	int fd[2];
}sip_msg_t;

extern sip_msg_t sip_msg;

extern void *Kernel_sip_pthread(void *arg);
extern void *Kernel_sipcmd_pthread (void *arg);

extern void Sip_released();

#endif 
