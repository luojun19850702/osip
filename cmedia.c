#include "config.h"
#include <event.h>
#include "cmedia.h"
#include "alsa_if.h"
#include "message.h"

int isSPK;
int traceId = 0;
int traceId2 = 0;
int traceId3 = 0;

int AudMode = SP_MODE_HANDSET;
unsigned short duaUnitSpVoip[10];
int duaUnitVoip[10];
int connectionId;
rtp_dtmf_event dtmfParam;

#define STEP_SIZE 0x2 // to increase or decrease Volume by 6dB

x_SP_PhoneInfo vxPhoneInfo;

OPUS_CODEC_PARAM opus_codec_param;

const  t_da_Note beepMdy[] = {
	{0xEA3E, 0x1000, DA_MS2PARAM(500)},
	{0xEA3E, 0, DA_MS2PARAM(500)},						  	
	{1, 1, 1}
};	

#define COMEBACK_LATER 0

void Init_opus_configuration(void){
#if defined(UE_OPUSE)
#if defined(UT_WBHF) || defined(UT_BLWBHF)
	opus_codec_param.bVadFlag = 0;
	opus_codec_param.nSamplingRate = OPUS_SAMPLING_RATE_16k;
	opus_codec_param.nBandwidth = DSPA_OPUSE_BANDWIDTH_WIDEBAND;
	opus_codec_param.nMaxFrameSamples = 320;
	opus_codec_param.nFrameSamples = 320;
	opus_codec_param.nBitRate = 20000;
	opus_codec_param.nComplexity = 5;
	opus_codec_param.bVbr = 1;
	opus_codec_param.bVbrConstraint = 0;
	opus_codec_param.nMaxDataBytes = 2500;
	opus_codec_param.nEncOpusMode = DSPA_OPUSE_MODE_SILK_ONLY;
	opus_codec_param.bFec = 0;
	opus_codec_param.nPacketLossPercentage = 0;
#elif defined(UT_SWBHF)
	opus_codec_param.bVadFlag = 0;
	opus_codec_param.nSamplingRate = OPUS_SAMPLING_RATE_48k;
	opus_codec_param.nBandwidth = DSPA_OPUSE_BANDWIDTH_FULLBAND;
	opus_codec_param.nMaxFrameSamples = 960;
	opus_codec_param.nFrameSamples = 960;
	opus_codec_param.nBitRate = 64000;
	opus_codec_param.nComplexity = 5;
	opus_codec_param.bVbr = 1;
	opus_codec_param.bVbrConstraint = 0;
	opus_codec_param.nMaxDataBytes = 2500;
	opus_codec_param.nEncOpusMode = DSPA_OPUSE_MODE_HYBRID;
	opus_codec_param.bFec = 0;
	opus_codec_param.nPacketLossPercentage = 0;
#endif
#endif
}

void SP_Mute(int mute, int id){
	int ret = 0;
	if (SP_MUTE == mute ) {
		ret = p_duasync_UnitSetReq(id,UE_OPX, DUA_PARAM_PIN_MUTE, INT2PV(DUA_MUTE_ON),0);
		if(ret < 0)
			printf("ERR: SP MUTE p_duasync_UnitSetReq Failed\n");
	}  else if (SP_UNMUTE == mute){
		ret = p_duasync_UnitSetReq(id,UE_OPX, DUA_PARAM_PIN_MUTE, INT2PV(DUA_MUTE_OFF),0);
		if(ret < 0)
			printf("ERR: SP UNMUTE p_duasync_UnitSetReq Failed\n");	
	}  else {
		printf(" Invalid Mode in SP_Mute\n");
	}
}

void AlsaSwitchOffMode(int mode){
	printf("alsaSwitchOffMode: switching mode %d off\n", mode);
	switch (mode){
		case SP_MODE_HANDSET:
			// Nothing to do
			break;
		case SP_MODE_HEADSET:
			// Nothing to do
			break;
		case SP_MODE_SPEAKER:
			dvf99_snd_ctl_write("DCLASS_EN", "0", ALSA_CARD_CSS);
			break;
		default:
			printf(" Invalid Mode in alsaSwitchOffMode\n");
			break;
	}
}

int AlsaSwitchOnMode(int mode){
	int ret = 0;
	printf("alsaSwitchOnMode: switching mode %d on\n", mode);

	switch (mode)
	{
		case SP_MODE_HANDSET:
			ret = p_duascsync_UnitSetReq(UT_STR_WBHF, UNITINST_OF_DUA_UID(vxPhoneInfo.u32SpkUnitId), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", UT_STR_WBHF_MODE_HANDSET, 0);
			if (ret < 0) {
				printf(" setWbhfAudMode - SP_MODE_HANDSET - p_duascsync_UnitSetReq Failed\n");
			}

			dvf99_snd_ctl_write("Input Mux0", "DIFFIN1",ALSA_CARD_CSS);
			// Playback
			dvf99_snd_ctl_write("Output0_diff Gain", "1",ALSA_CARD_CSS);// Handset Playback gain
			dvf99_snd_ctl_write("Output1_diff Gain", "1",ALSA_CARD_CSS);// Handset Playback gain
			//capture
			dvf99_snd_ctl_write("DIFFIN1 Gain", "18",ALSA_CARD_CSS); // Handset Capture gain
			dvf99_snd_ctl_write("DAC0_CURRENT_SCALE", "0dB",ALSA_CARD_CSS);
			break;

		case SP_MODE_HEADSET:
			ret = p_duascsync_UnitSetReq(UT_STR_WBHF, UNITINST_OF_DUA_UID(vxPhoneInfo.u32SpkUnitId), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", UT_STR_WBHF_MODE_HANDSFREE, 0);
			if (ret < 0) {
				printf(" setWbhfAudMode - SP_MODE_HEADSET - p_duascsync_UnitSetReq Failed\n");
			}
			dvf99_snd_ctl_write("Input Mux0", "DIFFIN0",ALSA_CARD_CSS);
			//Playback
			dvf99_snd_ctl_write("Output2_diff Gain", "1",ALSA_CARD_CSS);// Headset Playback gain
			dvf99_snd_ctl_write("Output3_diff Gain", "1",ALSA_CARD_CSS);// Headset Playback gain

			// Capture
			dvf99_snd_ctl_write("DIFFIN0 Gain", "18",ALSA_CARD_CSS); // Headset Capture gain

			dvf99_snd_ctl_write("DAC1_CURRENT_SCALE", "0dB",ALSA_CARD_CSS);
			break;

		case SP_MODE_SPEAKER:
			ret = p_duascsync_UnitSetReq(UT_STR_WBHF, UNITINST_OF_DUA_UID(vxPhoneInfo.u32SpkUnitId), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", UT_STR_WBHF_MODE_D_CLASS, 0);
			if (ret < 0) {
				printf(" setWbhfAudMode - SP_MODE_SPEAKER - p_duascsync_UnitSetReq Failed\n");
			}

			dvf99_snd_ctl_write("DCLASS_EN", "1",ALSA_CARD_CSS);
			dvf99_snd_ctl_write("Input Mux0", "SINGIN0-SINGIN1",ALSA_CARD_CSS);

			dvf99_snd_ctl_write("SINGIN0 Gain", "10",ALSA_CARD_CSS); // Speaker Capture gain
			dvf99_snd_ctl_write("SINGIN1 Gain", "10",ALSA_CARD_CSS); // Speaker Capture gain
			break;

		default:
			printf(" Invalid Mode in alsaSwitchOnMode\n");
			break;
	}

	return 0;    
}

int PutInHandsFreeMode(int SpkUnit){
	int ret = 0;
	if (isSPK){
		ret = p_duascsync_UnitSetReq(UT_STR_WBHF, UNITINST_OF_DUA_UID(SpkUnit), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", UT_STR_WBHF_MODE_D_CLASS, 0);
		if(ret < 0)
			printf("ERR: PutInHandsFreeMode : p_duascsync_UnitSetReq Failed\n");
	}
	return 0;
}

int EnableSPKUnit(int SpkUnit, int isWB){
	int ret=0;

	ret = p_duascsync_UnitSetReq("UT_WBHF", UNITINST_OF_DUA_UID(SpkUnit), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", "UT_WBHF_MODE_HANDSET", 0);
	if(ret < 0)
		printf("ERR:WBHF EnableSPKUnit p_duascsync_UnitSetReq Failed\n");
	printf("WBHF unit is initialized properly\n");
	return ret;
}

int DisableSPKUnit(int SpkUnit){
	int ret=0;
	ret = p_duascsync_UnitSetReq("UT_WBHF", UNITINST_OF_DUA_UID(SpkUnit), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", "UT_WBHF_MODE_IDLE", 0);
	if(ret < 0)
		printf("ERR: WBHF DisableSPKUnit p_duascsync_UnitSetReq Failed\n");
	// In the case of FXS, the Unit is enabled at startup itself
	return ret;
}

static int  allocTraceUnit(void){
	int ret = 0;
	//Allocating trace unit-1
	traceId = p_duascsync_UnitAllocateReq("UT_TRACE", 0);
	printf("\n Allocated PCm FD Unit-1 with WB config [%d] \n", traceId);
	ret = p_duascsync_UnitSetReq("UT_TRACE", UNITINST_OF_DUA_UID(traceId), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", "UT_TRACE_MODE_WB", 0);
	if(ret <0)
		printf("ERR:AllocTraceUnit 1 p_duascsync_UnitSetReq Failed\n");

	//Allocating trace unit-2
	traceId2 = p_duascsync_UnitAllocateReq("UT_TRACE", 1);
	printf("\n Allocated PCm FD Unit-2 with NB config [%d] \n", traceId2);
	ret = p_duascsync_UnitSetReq("UT_TRACE", UNITINST_OF_DUA_UID(traceId2), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", "UT_TRACE_MODE_NB", 0);
	if(ret <0)
		printf("ERR: AllocTraceUnit 2 p_duascsync_UnitSetReq Failed\n");

	//Allocating trace unit-3
	traceId3 = p_duascsync_UnitAllocateReq("UT_TRACE", 2);
	printf("\n Allocated PCm FD Unit-3 with SWB config [%d] \n", traceId3);
	ret = p_duascsync_UnitSetReq("UT_TRACE", UNITINST_OF_DUA_UID(traceId3), "DUA_ANY", "DUA_PARAM_UMT_EXEC_GEN", "UT_TRACE_MODE_SWB", 0);
	if(ret <0)
		printf("ERR: AllocTraceUnit 3 p_duascsync_UnitSetReq Failed\n");

	return traceId;
}

void connect_wbhf_voipline(int line){
	int ret;
	printf("connect_wbhf_voipline : line = %d\n",line);
	printf("connectionId = p_duasync_UnitConnectReq(%d, 0);\n", vxPhoneInfo.u32SpkUnitId);
	connectionId = p_duasync_UnitConnectReq(vxPhoneInfo.u32SpkUnitId, DUA_NONE);
	printf("p_duasync_UnitConnectReq returned %d\n", connectionId);		
	if (connectionId  < 0 )
	{
		printf("Error..\n");
	}
	printf("connecting Unit 0x%x from Connection 0x%x \n\n", vxPhoneInfo.u32SpkUnitId, 
			connectionId);

	if(duaUnitVoip[line] != 0)
	{

		printf("ret = p_duasync_UnitConnectReq (%d, %d);\n", duaUnitVoip[line], connectionId);
		ret = p_duasync_UnitConnectReq (duaUnitVoip[line], connectionId);
		printf("p_duasync_UnitConnectReq returned %d\n", ret);
		if (ret < 0 )
			printf("Error..\n");
	} 
	else 
	{
		/* Coding guide lines */
	}



	EnableSPKUnit (vxPhoneInfo.u32SpkUnitId,0);
	//printf("connecting Unit 0x%x from Connection 0x%x \n\n", duaUnitVoip[line], 
	//connectionId);

}

void disconnect_wbhf_voipline(int line){
	int ret=0;
	printf("disconnect_wbhf_voipline : line = %d\n",line);
	ret = p_duasync_UnitDisconnectReq(vxPhoneInfo.u32SpkUnitId, connectionId);

	if (ret < 0) {
		printf(" p_duasync_UnitDisconnectReq Failed\n");
	}

	printf("disconnecting Unit 0x%x from Connection 0x%x \n\n", vxPhoneInfo.u32SpkUnitId, 
			connectionId);

	printf("p_duasync_UnitDisconnectReq returned %d\n",ret);

	ret = p_duasync_UnitDisconnectReq(duaUnitVoip[line], connectionId);
	if (ret < 0) {
		printf(" p_duasync_UnitDisconnectReq Failed\n");
	}

	printf("disconnecting Unit 0x%x from Connection 0x%x \n\n", duaUnitVoip[line], 
			connectionId);

	printf("p_duasync_UnitDisconnectReq returned %d\n",ret);

	ret = p_duasync_ConnDeleteReq(connectionId);
	if (ret < 0) {
		printf("p_duasync_ConnDeleteReq Failed 2\n");
	}

	printf("p_duasync_ConnDeleteReq returned %d\n",ret);

	DisableSPKUnit (vxPhoneInfo.u32SpkUnitId);
}

int Uninit_Cmedia (void){
	int ret;int line;
#ifdef LOCAL_HS_SUPPORT
	ret = p_duasync_UnitSetReq(vxPhoneInfo.u32SpkUnitId, DUA_ANY, DUA_PARAM_UMT_EXEC_GEN, INT2PV(UT_X_WBHF_MODE_IDLE), 0);
	if (ret < 0)
	{
		printf("Error.. p_duasync_UnitSetReq Failed\n");
	}
#endif
	printf("ret = p_duasync_UnitFreeReq(%d);\n", vxPhoneInfo.u32SpkUnitId);
	ret = p_duasync_UnitFreeReq(vxPhoneInfo.u32SpkUnitId);
	printf("p_duasync_UnitFreeReq returned %d\n", ret);
	if (ret < 0)
	{
		printf("Error.. p_duasync_UnitFreeReq Failed\n");
	}

	line = 0;
	while(line < MAX_CALLS_SUPPORTED){
		if(line < UT_X_VOIP_NINST){
			// Set VOIP_MODE to IDLE
			ret = p_duasync_UnitSetReq(duaUnitVoip[line], DUA_ANY, DUA_PARAM_UMT_EXEC_GEN, INT2PV(UT_X_VOIP_MODE_IDLE), 0);
			if (ret < 0){
				printf("Error.. p_duasync_UnitSetReq Failed\n");
			}

			// Free
			printf("ret = p_duasync_UnitFreeReq(%d);\n",duaUnitVoip[line]);
			ret = p_duasync_UnitFreeReq(duaUnitVoip[line]);
			printf("p_duasync_UnitFreeReq returned %d\n", ret);
			if (ret < 0){
				printf("Error.. p_duasync_UnitFreeReq Failed\n");
			}
		}
		line ++;
	}

	//daif_disable();
	printf("alsa_close();");
	alsa_close();
	return 1;
}

int Init_Cmedia (void){
	int ret;
	Init_opus_configuration ();	
	snd_pcm_open (NULL,NULL,SND_PCM_STREAM_PLAYBACK, 0);
/**********************************************************************/
	printf("duasync_init();\n");
	ret = duasync_init();
	printf("duasync_init returned %d\n", ret);
/**********************************************************************/
	if ((dvf99_snd_ctl_open(ALSA_CARD_CSS)) < 0) {
		printf("ERR: dvf99_snd_ctl_open failed!\n");
		return 0;
	} else{
		printf("dvf99_snd_ctl_open succeeded!\n");
	}

#ifdef UT_SWBHF
	printf("Inside %s at line %d Disabling the ULTRA_WB\n",__FUNCTION__,__LINE__);
	dvf99_snd_ctl_write("ULTRA_WB_EN", "0",ALSA_CARD_CSS);
	printf("Inside %s at line %d Enabling the ULTRA_WB\n",__FUNCTION__,__LINE__);
	dvf99_snd_ctl_write("ULTRA_WB_EN", "1",ALSA_CARD_CSS);
#endif
	
	//Start trace unit
	allocTraceUnit();

#ifdef LOCAL_HS_SUPPORT
	// Initialize dua unit WBHF
	printf("p_duasync_UnitAllocateReq(UT_X_WBHF, 0)\n");
	vxPhoneInfo.u32SpkUnitId = p_duasync_UnitAllocateReq(UT_X_WBHF, 0);
	printf("p_duasync_UnitAllocateReq returned %d\n", vxPhoneInfo.u32SpkUnitId);
	if (vxPhoneInfo.u32SpkUnitId < 0)
	{
		printf("Error.. WBHF allocation Failed\n");
		return -1;
	}
#endif
	
	p_duascsync_UnitSetReq(UT_STR_WBHF, UNITINST_OF_DUA_UID(vxPhoneInfo.u32SpkUnitId), "UE_EAC", "DUA_PARAM_ISWITCH", INT2PV(0x14), 0);
	p_duascsync_UnitSetReq(UT_STR_WBHF, UNITINST_OF_DUA_UID(vxPhoneInfo.u32SpkUnitId), "UE_C2S", "DUA_PARAM_ISWITCH", INT2PV(0), 0);
	p_duasync_UnitSetReq(vxPhoneInfo.u32SpkUnitId,UE_OPX, DUA_PARAM_PIN_MUTE, INT2PV(DUA_MUTE_OFF),0);

	// Set a mode to WBHF
	printf("setAudMode(%d);\n", AudMode);
	/*ret = AlsaSwitchOnMode(SP_MODE_SPEAKER);
	vxPhoneInfo.eAudioMode = SP_MODE_SPEAKER;*/
	ret = AlsaSwitchOnMode(SP_MODE_HANDSET);
	vxPhoneInfo.eAudioMode = SP_MODE_HANDSET;
	printf("setAudMode returned %d\n", ret);
	if (ret < 0 )
		printf("Error..\n");


		// Initialize VoIP units

	int line = 0;
	vxPhoneInfo.ePhoneState = SP_STATE_IDLE;			//
	isSPK = 0;									//
	while(line < MAX_CALLS_SUPPORTED){
		if(line < UT_X_VOIP_NINST){
			printf("duaUnitVoip = p_duasync_UnitAllocateReq(%d, %d);\n", UT_X_VOIP, line);
			duaUnitVoip[line] = p_duasync_UnitAllocateReq(UT_X_VOIP, line);
			printf("p_duasync_UnitAllocateReq returned %d\n", duaUnitVoip[line]);
			if (duaUnitVoip[line] < 0){
					printf(" Error..VOIP unit allocation Failed\n");
					return -1;
			}
		}
		line ++;
	}
	return 1;
}

void playTone(int SpkUnit)
{
	static const t_da_Note* pMelos[1] = {beepMdy};
	int v0dB, ret, nMelos;
	printf("called SP_PlayTone\n");

	nMelos= 1;

	v0dB = 1536;
	ret = p_duasync_PlayMelodyReq(SpkUnit, UE_IPX, nMelos, pMelos, v0dB);
	if(ret <0)
		printf("ERR: playTone p_duasync_PlayMelodyReq Failed\n");

	printf("Return value of p_duasync_PlayMelodyReq %d \n", ret);
}

void mmi_TestPlayTone(void){
	if (vxPhoneInfo.ePhoneState == SP_STATE_IDLE) {
		if(isSPK == 1){
			SP_Mute(SP_UNMUTE, vxPhoneInfo.u32SpkUnitId);
			AlsaSwitchOnMode(vxPhoneInfo.eAudioMode);
		}
		if ((vxPhoneInfo.eAudioMode == SP_MODE_HEADSET) || (vxPhoneInfo.eAudioMode == SP_MODE_HANDSET)) {
			printf(" Enabling SPKUnit \n");
			EnableSPKUnit(vxPhoneInfo.u32SpkUnitId,0);
		}
		else if(vxPhoneInfo.eAudioMode == SP_MODE_SPEAKER){
			printf(" Enabling Handsfree \n");
			PutInHandsFreeMode(vxPhoneInfo.u32SpkUnitId);	
		}

	}
	playTone(vxPhoneInfo.u32SpkUnitId);
}

void stopTone(int SpkUnit){
	int ret;
	printf("called stopTone\n");
	// Need to implement sync api for this function
	ret = p_duasync_StopMelody(SpkUnit, UE_IPX);
	if(ret <0)
		printf("ERR: stopTone p_duasync_StopMelodyReq Failed\n");

	return;
}

void mmi_TestStopTone(){
	stopTone(vxPhoneInfo.u32SpkUnitId);
	if (vxPhoneInfo.ePhoneState == SP_STATE_IDLE) {
		if(isSPK==1){
			SP_Mute(SP_MUTE, vxPhoneInfo.u32SpkUnitId);
			AlsaSwitchOffMode(vxPhoneInfo.eAudioMode);
			DisableSPKUnit(vxPhoneInfo.u32SpkUnitId);
		}
	}
}

int ChangeSpkVolume(int SpkUnit, int isIncrease){
	s32 value=0;
	s32 value_array[2] = {0,0};
	s32 *array_value = value_array;
	int res = 0;
	if (vxPhoneInfo.eAudioMode == SP_MODE_SPEAKER){
		res = p_duasync_UnitGetReq(SpkUnit, UE_VX_C, DSPA_ADDR_OFFS_VX_VOL , array_value);
		if(res < 0){
			printf("ERR: V4_VOL p_duasync_UnitGetReq ERR: res = %d\n", res);
		}
	}
	else{
		res = p_duasync_UnitGetReq(SpkUnit, UE_VX_D, DSPA_ADDR_OFFS_VX_VOL , array_value);
		if(res < 0){
			printf("ERR: V4_VOL p_duasync_UnitGetReq ERR: res = %d\n", res);
		}
	}
	value = value_array[0];
	printf("BEFORE Volume data from V4_D & V4_C is 0x%x \n", value);

	/* Requirement: Maximum value of V4 Volume is 0x800 and minimum value is 0x0 */
	if (isIncrease) {

		if(value == 0){
			value = 0x40;
		}
		else if(value < 0x800){
			value *= STEP_SIZE; //increase Volume by 6dB
		}
	} else {

		if(value <= 0x40)
			value = 0;
		else
			value /= STEP_SIZE; //reduce Volume by 6dB

	}
	if (vxPhoneInfo.eAudioMode == SP_MODE_SPEAKER){
		res = p_duasync_UnitSetReq(SpkUnit, UE_VX_C, DSPA_ADDR_OFFS_VX_VOL, INT2PV(value), 0);
		if(res < 0)
			printf("ERR: p_duasync_UnitSetReq Failed for V4_C\n");
	}
	else{
		res = p_duasync_UnitSetReq(SpkUnit, UE_VX_D, DSPA_ADDR_OFFS_VX_VOL, INT2PV(value), 0);
		if(res < 0)
			printf("ERR: p_duasync_UnitSetReq Failed  for V4_D\n");
	}

	if( value == 0x800){
		printf("Adjust RX Volume is at MAX Value: V4_C & V4_D is 0x%x \n", value);
	}
	else if(value == 0x0){
		printf("Adjust RX Volume is at MIN Value: V4_C & V4_D is 0x%x \n", value);
	}
	else{
		printf("AFTER Adjust RX Volume data from V4_C & V4_D is 0x%x \n", value);
	}

	return 0;
}

int ChangeMicVolume(int SpkUnit, int isIncrease){
	s32 value=0;
	s32 value_array[2] = {0,0};
	s32 *array_value = value_array;
	int res = 0;
	res = p_duasync_UnitGetReq(SpkUnit, UE_OPX, DUA_PARAM_PIN_VOLUME, array_value);
	if(res < 0){
		printf("ERR: V4_VOL p_duasync_UnitGetReq ERR: res = %d\n", res);
	}
	value = value_array[0];
	printf("BEFORE Volume data from OPI is %x \n", value);
	if (isIncrease) {
		if(value == 0){
			value = 0x40;
		}
		else if(value < 0x800){
			value *= STEP_SIZE; //increase Volume by 6dB
		}
	} else {

		if(value <= 0x40)
			value = 0;
		else
			value /= STEP_SIZE; //reduce Volume by 6dB

	}
	res = p_duasync_UnitSetReq(SpkUnit, UE_OPX, DUA_PARAM_PIN_VOLUME, INT2PV(value), 0);
	if(res <0)
		printf("ERR: ChangeMicVolume : p_duasync_UnitSetReq Failed\n");

	printf("After Change, Volume data from OPI is %x \n", value);
	return 0;
}

void mmi_TestPlayDTMFToLine(void){
	t_dua_UID   uid;
	int v0dB,ret;
	static const u8 dialDigits[]= { DA_DTMF_0 
		,   DA_DTMF_1
			,   DA_DTMF_2
			,   DA_DTMF_3
			,   DA_DTMF_4
			,   DA_DTMF_5
			,   DA_DTMF_6
			,   DA_DTMF_7
			,   DA_DTMF_8
			,   DA_DTMF_9
			,   DA_DTMF_PAUSE
			,   DA_DTMF_A
			,   DA_DTMF_B
			,   DA_DTMF_C
			,   DA_DTMF_D
			,   DA_DTMF_HASH
			,   DA_DTMF_STAR};
	uid= DUA_UID(UT_X_WBHF, 0);	

	if ((vxPhoneInfo.eAudioMode == SP_MODE_HEADSET) || (vxPhoneInfo.eAudioMode == SP_MODE_HANDSET)) {
		printf(" Enabling SPKUnit \n");
		EnableSPKUnit(vxPhoneInfo.u32SpkUnitId,0);
	}
	else if(vxPhoneInfo.eAudioMode == SP_MODE_SPEAKER){
			printf(" Enabling Handsfree \n");
		PutInHandsFreeMode(vxPhoneInfo.u32SpkUnitId);
	}
	v0dB = 1536;
	ret = p_duasync_DialDTMFReq(uid, UE_IPX, 1, &dialDigits[1], v0dB);
}

static void Cmedia_cmd_mod (int fd, short event, void* arg){
	char cmd_msg[MESSAGE_MAX_LEN] = {0};
	message_t message_media;
	if (event & EV_READ){
		read (fd,cmd_msg,sizeof(cmd_msg));
		if (Recive_media_message (cmd_msg,&message_media) == 1){
			switch (message_media.cmd){
				case MEDIA_VOL_UP:{
					printf ("rev MEDIA_VOL_UP cmd !!\n");
					ChangeSpkVolume (vxPhoneInfo.u32SpkUnitId,1);
				}break;
				case MEDIA_VOL_DOWN:{
					printf ("rev MEDIA_VOL_DOWN cmd !!\n");
					ChangeSpkVolume (vxPhoneInfo.u32SpkUnitId,0);
				}break;
				case MEDIA_PLAY_TONE:{
					printf ("rev MEDIA_PLAY_TONE cmd !!\n");
					//Dtmf_send_to_line ();
					//mmi_TestPlayDTMFToLine ();
					mmi_TestPlayTone ();
				}break;
				case MEDIA_STOP_TONE:{	
					printf ("rev MEDIA_STOP_TONE cmd !!\n");	
					mmi_TestStopTone();		
				}break;
				case MEDID_DTMF_ON_Line:{
					printf ("rev MEDID_DTMF_ON_Line cmd !!\n");
					//mmi_TestPlayDTMFToLine ();
				}break;
				default:{
				}break;
			}
		}
	}
}

void *Kernel_cmedia_pthread (void *arg){
	struct event* mediacmd_ev;
	struct event_base* media_base;
	media_base = event_base_new();
	while (1){
		mediacmd_ev = event_new(media_base,media_msg.fd[PIPE_READ],EV_READ|EV_PERSIST,Cmedia_cmd_mod,NULL);
		event_add(mediacmd_ev, NULL);
		event_base_dispatch(media_base);
		sleep(1);
	}
	return(void *)0;
}
