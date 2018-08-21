#include "config.h"
#include "alsa_man.h"

#define PCM_WB_10MS_FRAMESIZE 320

#define MAX_READ	10

static snd_pcm_uframes_t  period_size, buffer_size;
static snd_pcm_t *handlePlayHS = NULL;
char * alsa_play_devices[] = {"CssPlayback", "CssPlayback_Headset", "CssPlayback_Spk", "CssPlayback_Usb"};

static int Set_playback_hw_param(snd_pcm_t *handlePlay);

static int get_hw_params(snd_pcm_t *handle)
{
	snd_pcm_hw_params_t *params;
	int dir;
	snd_pcm_hw_params_alloca(&params);
	if( snd_pcm_hw_params_current (handle, params) < 0)
	{
		printf(" Get current hw params failed\n");
		return -1;
	}
	snd_pcm_hw_params_get_period_size(params, &period_size, &dir);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	return 0;
}

int setparams_set(snd_pcm_t *handle, char *id)
{
        int err;
        snd_pcm_uframes_t val;
	 	snd_pcm_hw_params_t *params;
	 	snd_pcm_sw_params_t *swparams;

        snd_pcm_hw_params_alloca(&params);
        snd_pcm_sw_params_alloca(&swparams);
        err = snd_pcm_hw_params_current(handle, params);
        if (err < 0) {
                printf("Unable to get hw params for %s: %s\n", id, snd_strerror(err));
                return err;
        }
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for %s: %s\n", id, snd_strerror(err));
                return err;
        }
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 481);
        if (err < 0) {
                printf("Unable to set start threshold mode for %s: %s\n", id, snd_strerror(err));
                return err;
        }

        snd_pcm_hw_params_get_period_size(params, &val, NULL);
		printf(" period size %d\n", (int) val);
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, val);
        if (err < 0) {
                printf("Unable to set avail min for %s: %s\n", id, snd_strerror(err));
                return err;
        }
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for %s: %s\n", id, snd_strerror(err));
                return err;
        }
        return 0;
}

int Init_alsa_man (int mode){
	int rc;
	rc = snd_pcm_open(&handlePlayHS, alsa_play_devices[mode],SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		printf(" Error opening the PCm Playback device \n");
		return -1;
	}
	printf(" Playback open success handle %x\n" ,(int)handlePlayHS);
	if(Set_playback_hw_param(handlePlayHS) < 0){
		printf(" set_playback_hw_param Failed for alsa_playback_HS_open\n");
		return -1;
	}
	get_hw_params(handlePlayHS);
	setparams_set(handlePlayHS,"playback_Headset");
	return 1;
}

void Uninit_alsa_man (void){
	snd_pcm_close(handlePlayHS);
	handlePlayHS = NULL;
}

static int Set_playback_hw_param(snd_pcm_t *handlePlay){
	snd_pcm_hw_params_t *params;
	unsigned int val;
	int rc;
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handlePlay, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handlePlay, params, 
				SND_PCM_ACCESS_RW_INTERLEAVED);

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handlePlay, params,
			      SND_PCM_FORMAT_S16_LE);

	/* Single channel (mono) */
	snd_pcm_hw_params_set_channels(handlePlay, params, 1);

	/* 44100 bits/second sampling rate (CD quality) */
	val = 16000;
	snd_pcm_hw_params_set_rate_near(handlePlay,
				 params, &val, NULL);

	val = 4000;
//	snd_pcm_hw_params_set_period_size(handlePlay, params, val,NULL);
	snd_pcm_hw_params_set_period_size(handlePlay, params, val,0);

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handlePlay, params);
	if (rc < 0) {
		printf("unable to set hw parameters: %s\n",snd_strerror(rc));
		return -1;
	}
	return 0;
}

int alsa_write(unsigned char *buf, int size){
	int ret =0;
	ret = snd_pcm_writei(handlePlayHS, buf, size);
	if(ret < 0){
		printf(" Write failed %d r=%s @\n",ret, snd_strerror(ret));
		snd_pcm_prepare(handlePlayHS);
	}
	return ret;
}

void Read_file (void){
	int fd;unsigned char buff[PCM_WB_10MS_FRAMESIZE]= {0};
	int len;
	fd = open("auto8.wav", O_RDWR);
	printf ("read file test fd is %d\n",fd);
	if (lseek(fd, 0, SEEK_CUR) == -1){
		printf ("lseek is error !!\n");
	}
	while (1){
		memset (buff,0x00,sizeof(buff));
		len = read (fd,buff,(PCM_WB_10MS_FRAMESIZE/2));
		printf ("read len is %d\n",len);
		alsa_write (buff,len);
		if (len < (PCM_WB_10MS_FRAMESIZE/2)){
			break;
		}
		
	}	
	printf ("read file test is end!!\n");
}


void *Alsa_cmedia_pthread (void *arg){
	Init_alsa_man (2);
	Read_file ();
	Uninit_alsa_man();
	while (1){
		sleep(1);
	}
}




