#ifndef CMEDIA_H
#define CMEDIA_H

#define SP_MUTE 		1
#define SP_UNMUTE 		0

typedef enum{
	MEDIA_VOL_UP,
	MEDIA_VOL_DOWN,
	MEDIA_PLAY_TONE,
	MEDIA_STOP_TONE,
	MEDID_DTMF_ON_Line,
}media_cmd;

typedef enum{
	SP_MODE_HANDSET,
	SP_MODE_HEADSET,
	SP_MODE_SPEAKER
} e_SP_Mode;

typedef enum
{
	SP_STATE_IDLE,  /*! On hook state */
	SP_STATE_CONNECTED, /*! Off hook state */
}e_SP_SPK_State;


typedef struct media_msg_str{
	int fd[2];
}media_msg_t;

/*! \ Data structure which has phone information. */
typedef struct sx_SP_PhoneInfo
{
	e_SP_SPK_State ePhoneState; /* ! Phone status. */
	u8   u8NoOfCalls;                    /* !Either in hold or active. */
	s32 u32SpkUnitId;                  /* ! Speaker DUA unit. */
       u8   u8NumCallsInConf;          /* ! Number of lines in conference. */
	u32 u32connectionId;             /* ! Dua connection ID. */
	e_SP_Mode eAudioMode;					/* Whether the mode is Handset(0) or Handsfree(1) */
}x_SP_PhoneInfo;

media_msg_t media_msg;

extern int duaUnitSpk;

extern int Init_Cmedia (void);
extern int Uninit_Cmedia (void);
extern void connect_wbhf_voipline(int line);
extern void disconnect_wbhf_voipline(int line);

extern int DisableSPKUnit(int SpkUnit);
extern int EnableSPKUnit(int SpkUnit, int isWB);

extern void *Kernel_cmedia_pthread (void *arg);

#endif
