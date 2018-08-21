#include "config.h"
#include <linux/input.h>
#include <linux/prctl.h>
#include "event.h"
#include "key_man.h"
#include "cmedia.h"
#include "message.h"

#define KEY_PRESS 			1
#define KEY_VOL_DOWN 		113
#define KEY_VOL_UP			114
#define KEY_TONE_ON			158
#define KEY_TONE_OFF		234

static void Keyscan_mod (int fd, short event, void* arg){
	struct input_event ev;
	size_t rb = read(fd, &ev, sizeof(ev));
	if (rb < (int) sizeof(struct input_event)){
		perror("short read");
		printf("short read\n");
		return;
	}
	if ((EV_KEY == ev.type)&&(ev.value == KEY_PRESS)){ 	
		printf ("ev.code is %d,ev.value is %d\n",ev.code,ev.value);
		switch (ev.code){
			case KEY_VOL_UP:{
				Send_media_message (media_msg.fd[PIPE_WRITE],MEDIA_VOL_UP,NULL,NULL,0);
			}break;
			case KEY_VOL_DOWN:{
				Send_media_message (media_msg.fd[PIPE_WRITE],MEDIA_VOL_DOWN,NULL,NULL,0);
			}break;
			case KEY_TONE_ON:{
				Send_media_message (media_msg.fd[PIPE_WRITE],MEDIA_PLAY_TONE,NULL,NULL,0);
			}break;
			case KEY_TONE_OFF:{
				Send_media_message (media_msg.fd[PIPE_WRITE],MEDIA_STOP_TONE,NULL,NULL,0);
			}break;
			default:{
			}break;
		}
	}
}

void *Kernel_key_pthread(void *arg){
	int key_fd= 0;
	struct event_base* key_base;
	struct event* keyscan_ev;

	if ((key_fd = open("/dev/input/event0", O_RDONLY)) < 0) {
		perror("Couldn't open input device");
		printf("Couldn't open input device");
		return(void *)0; 
	}
	key_base = event_base_new();
	while (1){
		keyscan_ev = event_new(key_base,key_fd,EV_READ|EV_PERSIST,Keyscan_mod,NULL);
		event_add(keyscan_ev, NULL);
		event_base_dispatch(key_base);
		sleep (1);
	}
	return(void *)0; 
}






