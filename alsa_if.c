/**********************************************************************
*
*   C %name:        alsa_if.c %
*   Instance:       BLRPHNX_1
*   Description:    
*   %created_by:    Santosh K %
*   %date_created:  Wed Jan 16 18:16:20 2013 %
*
**********************************************************************/
#include "common.h"
#include <alsa/asoundlib.h>

//static char* aszCapDev[] = {"CssCapture", "CssCapture_Headset", "CssCapture_Spk"};
//static char* aszPlayDev[]    = {"CssPlayback", "CssPlayback_Headset", "CssPlayback_Spk"};

static char css_device[] = "CssDevice";

snd_pcm_t *handlePlay = NULL, *handleCap =NULL, *handleFXS;

int set_playback_hw_param(snd_pcm_t *handlePlay);
int get_hw_params(snd_pcm_t *handle);

#ifdef USE_ALSA

int alsa_open(int mode)
{
        int rc;
        snd_pcm_hw_params_t *params;
        snd_pcm_uframes_t periodSize;
        unsigned int val;

        /* Open PCM device for playback. */
        printf(" Opening playback device=%s\n", css_device); // Only CssPlayback is used
        rc = snd_pcm_open(&handlePlay, css_device,
                    SND_PCM_STREAM_PLAYBACK, 0);
        if (rc < 0) {
                fprintf(stderr,
            "unable to open pcm Playback device: %s\n",
            snd_strerror(rc));
                printf(" Error opening the PCm Playback device \n");
                return -1;
        }
        printf(" Playback open success handle %x\n" ,(int)handlePlay);
        if(set_playback_hw_param(handlePlay) < 0)
        {
                printf(" set_playback_hw_param Failed for \
                        alsa_playback_HS_open\n");
                return -1;
        }

        printf(" Opening capture device=%s\n", css_device); // Only CssCapture is used
        rc = snd_pcm_open(&handleCap, css_device,
                    SND_PCM_STREAM_CAPTURE, 0);
        if (rc < 0) {
                fprintf(stderr,
                    "unable to open pcm device: %s\n",
                    snd_strerror(rc));
                printf(" Error opening the PCm Capture device \n");
                return -1;
        }

        printf(" Capture open success handle %x\n" ,(int)handleCap);
        get_hw_params(handlePlay);
	
	
        snd_pcm_hw_params_alloca(&params);

        snd_pcm_hw_params_current (handleCap, params);
        snd_pcm_hw_params_get_period_size(params, &periodSize, NULL);
        printf("alsa_open: Current  period size for capture %d \n", (int)periodSize);


        /* Fill it in with default values. */
        snd_pcm_hw_params_any (handleCap, params);

        /* Set the desired hardware parameters. */

        /* Interleaved mode */

        /* Signed 16-bit little-endian format */
        snd_pcm_hw_params_set_format(handleCap, params,
                              SND_PCM_FORMAT_S16_LE);

        /* Single channel (mono) */
        snd_pcm_hw_params_set_channels(handleCap, params, 1);

        /* 44100 bits/second sampling rate (CD quality) */
        val = 16000;
        snd_pcm_hw_params_set_rate_near(handleCap,
                                 params, &val, NULL);

        /* Write the parameters to the driver */
        rc = snd_pcm_hw_params(handleCap, params);
        if (rc < 0) {
        fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
        return -1;
        }

        return 0;
}

void alsa_close()
{
        if(handlePlay)
        {
                printf(" Closing play back handle\n");
                snd_pcm_close(handlePlay);
        }
        if(handleCap)
                snd_pcm_close(handleCap);
        handlePlay = handleCap  = NULL;
}


int set_playback_hw_param(snd_pcm_t *handlePlay)
{
	snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir,rc;
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handlePlay, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handlePlay, params,
			      SND_PCM_FORMAT_S16_LE);

	/* Single channel (mono) */
	snd_pcm_hw_params_set_channels(handlePlay, params, 1);

	/* 44100 bits/second sampling rate (CD quality) */
	val = 8000;
	snd_pcm_hw_params_set_rate_near(handlePlay,
				 params, &val, &dir);

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handlePlay, params);
	if (rc < 0) {
	fprintf(stderr,
	    "unable to set hw parameters: %s\n",
	    snd_strerror(rc));
	return -1;
	}
	return 0;
}

int get_hw_params(snd_pcm_t *handle)
{
        snd_pcm_hw_params_t *params;
        
	snd_pcm_hw_params_alloca(&params);
        if( snd_pcm_hw_params_current (handle, params) < 0)
        {
                printf(" Get current hw params failed\n");
                return -1;
        }
        //snd_pcm_hw_params_get_period_size(params, &period_size, &dir);
        //snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
        return 0;
}



#endif


int alsa_open_fxs(){
	int rc;
	snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir;

	rc = snd_pcm_open(&handleFXS, "si32178Play",
		    SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		fprintf(stderr, "alsa_open_fxs: unable to open pcm device: %s\n",
				snd_strerror(rc));
		printf(" Error opening the PCm Playback device si32178Play\n");
		return -1;
	}
	printf(" Playback open success handle %x\n" ,(unsigned int)handleFXS);

	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handleFXS, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handleFXS, params,
			      SND_PCM_FORMAT_S16_LE);

	/* Single channel (mono) */
	snd_pcm_hw_params_set_channels(handleFXS, params, 1);

	/* 44100 bits/second sampling rate (CD quality) */
	val = 8000;
	snd_pcm_hw_params_set_rate_near(handleFXS,
				 params, &val, &dir);

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handleFXS, params);
	if (rc < 0) {
	fprintf(stderr,
	    "unable to set hw parameters: %s\n",
	    snd_strerror(rc));
	return -1;
	}

	return 0;
}

void alsa_close_fxs()
{
	if(handleFXS)
		snd_pcm_close(handleFXS);
	handleFXS = NULL;
}


