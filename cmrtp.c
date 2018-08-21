#include "config.h"
#include <sys/prctl.h>
#include "cmrtp.h"
#include "cmedia.h"

#define COMEBACK_LATER 0

//#ifdef USE_ALSA
#include "alsa_if.h"
//#endif
#define MY_AP  AP_SIP
#define DST_AP AP_SPKPHONE

#define PORT 8000

rtp_session_config init_rtp_session_config;
int previous_voip_line_id;

void Init_codec_configuration(){   
	int i;
	init_rtp_session_config.codec.tx_pt = CODEC_G711_MLAW;
	init_rtp_session_config.codec.rx_list[0].rx_pt = CODEC_G711_MLAW;
	strcpy(init_rtp_session_config.codec.rx_list[0].CodecStr,STR_CODEC_PCMU);
	init_rtp_session_config.codec.duration = 20;
	strcpy(init_rtp_session_config.codec.CodecStr,STR_CODEC_PCMU);
	init_rtp_session_config.codec.ssrc = 0; // librtp will populate with random value 
	init_rtp_session_config.codec.Timestamp = 0; // librtp will populate with random value 	
	init_rtp_session_config.codec.rx_pt_event = 101;
	init_rtp_session_config.codec.tx_pt_event = 101;

	init_rtp_session_config.codec.cng.level_rx=10;
	init_rtp_session_config.codec.cng.level_tx=10;
	init_rtp_session_config.codec.cng.max_sid_update=200;
	init_rtp_session_config.codec.cng.mode_rx=0;
	init_rtp_session_config.codec.cng.mode_tx=0;
	init_rtp_session_config.codec.cng.vad_detect_level=40;
	init_rtp_session_config.codec.cng.vad_hangover=50;

	init_rtp_session_config.codec.opts =  RTP_SESSION_OPT_DTMF;
	init_rtp_session_config.codec.opts |= RTP_SESSION_OPT_USE_JIB; 


	init_rtp_session_config.codec.opts |= RTP_CODEC_OPT_ILBC_15K2;

	for( i = 1 ; i < MAX_NUM_OF_CODEC ; i++){
		init_rtp_session_config.codec.rx_list[i].rx_pt = CODEC_G711_MLAW;
		strcpy(init_rtp_session_config.codec.rx_list[i].CodecStr,STR_CODEC_PCMU);
	}
}

void Init_amr_wb_configuration(void){
	init_rtp_session_config.codec.vbrCodecParam.amrwb.octetalign = 1; //Yes octet align
	init_rtp_session_config.codec.vbrCodecParam.amrwb.bVadFlag = 0;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.nBitRate = RTP_CODEC_VBR_AMRWB_2385;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.nFrameSize = 20;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.modechangeperiod = 2;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.modechangecapability = 2;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.crc = 0;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.robustsorting = 0;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.interleaving = 0;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.maxred = 0;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.nModeSetOpts = 0;
	init_rtp_session_config.codec.vbrCodecParam.amrwb.bModeChangeNeighbor = 0;
}

void Init_jib_config(void){
	init_rtp_session_config.jib_config.max_len = 1000;
	init_rtp_session_config.jib_config.min_len = 120;
	init_rtp_session_config.jib_config.monitoring_interval = 1500;
	init_rtp_session_config.jib_config.post_adapt_step_size = 960;
	init_rtp_session_config.jib_config.post_adapt_step_size_m = 480;
	init_rtp_session_config.jib_config.slope = 24;
	init_rtp_session_config.jib_config.stepsize_reset_time = 60;
	init_rtp_session_config.jib_config.target_delay = 20;
	init_rtp_session_config.jib_config.Th_resync = 3;
	init_rtp_session_config.jib_config.type = RTP_JIB_TYPE_ADAPTIVE;
}

void Init_redundancy_configuration(){
	init_rtp_session_config.redundancy_config.rtp_redundancy_level = 3;
	init_rtp_session_config.redundancy_config.rtp_redundancy_mode = REDUNDANCY_DISABLE;
	init_rtp_session_config.redundancy_config.rtp_redundant_rx_ptype_audio = 121;
	init_rtp_session_config.redundancy_config.rtp_redundant_tx_ptype_audio = 121;
	init_rtp_session_config.redundancy_config.rtp_redundant_rx_ptype_dtmf = 121;
	init_rtp_session_config.redundancy_config.rtp_redundant_tx_ptype_dtmf = 121;
}

void Init_rtp_session_config (void){
	memset (&init_rtp_session_config,0,sizeof(rtp_session_config));
	init_rtp_session_config.dtmf2833numEndPackets = DTMF_END_PKT_CNT;
	init_rtp_session_config.opts = 0;
	init_rtp_session_config.audio_mode = RTP_MODE_ACTIVE;
	init_rtp_session_config.media_loop_level = RTP_LOOP_LEVEL_NONE;
	init_rtp_session_config.rtcp_mux = 0;
	init_rtp_session_config.lib_rtp_mode = RTP_APP_VOIP_KERNEL;
	Init_jib_config ();
	Init_codec_configuration ();
	init_rtp_session_config.sid_update = 0;
	init_rtp_session_config.rtpRetransmissionMode = 0;
	init_rtp_session_config.rtp_retransmission_buffer_size = 180;
	Init_redundancy_configuration ();
	init_rtp_session_config.rtp_ses_event_config.report_codec_payload_change = 5;
	init_rtp_session_config.voip_line_id = 1;
	init_rtp_session_config.session_id = 1;
}

void voip_rtpcallback (unsigned int iChannelId, t_rtp_event_response *msg_resp)
{
	/*if(msg_resp && msg_resp->iEventType  == RTP_EVENT_RX_RFC2833)
	{
		RTP_RX_EVENT  *pRtpEvent = &msg_resp->uxData.RtpEvent;
		if(pRtpEvent->EndBit)
		{
			printf("USPACE: 2833 End pkt recv Channel %d Event %X Volume %d Duration %d Mbit = %d Ebit = %d Rbit = %d TS = %u\n",
					iChannelId,pRtpEvent->bEvent,pRtpEvent->bVolume,pRtpEvent->wDuration, 
					pRtpEvent->MarkerBit, pRtpEvent->EndBit, pRtpEvent->ReservedBit, pRtpEvent->dwTs);
		}
		else
		{
			printf("USPACE: Evnt Channel %d Event %X Volume %d Duration %d Mbit = %d Ebit = %d Rbit = %d TS = %u\n",
					iChannelId,pRtpEvent->bEvent,pRtpEvent->bVolume,pRtpEvent->wDuration, 
					pRtpEvent->MarkerBit, pRtpEvent->EndBit, pRtpEvent->ReservedBit, pRtpEvent->dwTs);
		}
	}*/
	bool valid_rtp_session_event_info = false;
	printf (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>receive kernel msg >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	switch(msg_resp->iEventType){
		case RTP_EVENT_RX_FIRST_RTP_PACKET:
			valid_rtp_session_event_info = true;
			printf("Received First RTP Packet from Network\n");break;
		case RTP_EVENT_TX_FIRST_RTP_PACKET:
			valid_rtp_session_event_info = true;
			printf("Sent First RTP Packet to Network\n");break;
		case RTP_EVENT_RX_MARKER_BIT_SET:
			valid_rtp_session_event_info = true;
			printf("RTP_EVENT_RX_MARKER_BIT_SET for\n");break;
		case RTP_EVENT_RX_SSRC_CHANGE:
			valid_rtp_session_event_info = true;
			printf("RX_SSRC_CHANGE for\n");break;
		case RTP_EVENT_RX_PAYLOAD_CHANGE:
			valid_rtp_session_event_info = true;
			printf("RX_PAYLOAD_CHANGE for\n");break;
	}

	if(valid_rtp_session_event_info){
		RTP_SESSION_EVENT *pRtpSessionEvent = &msg_resp->uxData.RtpSessionEvent;
		printf(" \n \
				%20s : %15d \n \
				%20s : %15d \n \
				%20s : %15d \n \
				%20s : %15d \n \
				%20s : %15d \n \
				%20s : %15d \n \
				%20s : %15d \n \
				%20s : %15u \n \
				%20s : %15u \n \
				%20s : %15u \n \
				%20s : %15s \n", \
				"Version",\
				pRtpSessionEvent->bVersion, \
				"Padding",\
				pRtpSessionEvent->bPadding, \
				"Extension Bit",\
				pRtpSessionEvent->bExtension, \
				"Csrc count",\
				pRtpSessionEvent->bCsrcCount, \
				"Marker Bit",\
				pRtpSessionEvent->bMarkerbit, \
				"Payload Type",\
				pRtpSessionEvent->bPayload, \
				"Sequence Number",\
				pRtpSessionEvent->wSeq, \
				"Ssrc",\
				pRtpSessionEvent->dwSsrc, \
				"Timestamp",\
				pRtpSessionEvent->dwTs,\
				"PayloadLen",\
				pRtpSessionEvent->wPlLen,\
				"CodecStr",\
				pRtpSessionEvent->CodecStr);

		//if payload change event, disconnec the spkr unit, send update to CSS with
		// new payload type and string, then connect the spkr unit
		if( RTP_EVENT_RX_PAYLOAD_CHANGE == msg_resp->iEventType){
			printf("%s - RTP_EVENT_RX_PAYLOAD_CHANGE received, disconnecting wbhf_voipline\n",__FUNCTION__);
			if( previous_voip_line_id != init_rtp_session_config.voip_line_id){
				printf("CHANGE IN VOIP LINE ID - updating voip line id\n");
				previous_voip_line_id = init_rtp_session_config.voip_line_id;
			}
			//prepare rtp config struct for RTP update
			/*rtp_session_config rtp_config;
			int ret;
			memset(&rtp_config, 0, sizeof(rtp_config));
			disconnect_wbhf_voipline(previous_voip_line_id);
			DisableSPKUnit(duaUnitSpk);
			printf("%s - updating rx and tx pt\n",__FUNCTION__);
			set_codec_dynamic(pRtpSessionEvent->bPayload, &pRtpSessionEvent->CodecStr);
			memcpy(&rtp_config,&init_rtp_session_config,sizeof(rtp_session_config));
			printf("%s - Sending RTP update to CSS\n",__FUNCTION__);
			ret = rtp_session_update(init_rtp_session_config.session_id, &rtp_config);
			if( ret != RTP_APP_SUCCESS){
				printf("#####rtp_session_update FAILED for session id : %d reason : %d#######\n",init_rtp_session_config.session_id,ret);
				return ;
			}else{
				printf("#####rtp_session_update PASSED for session id : %d reason : %d#######\n",init_rtp_session_config.session_id,ret);
			}
			printf("%s - connecting wbhf_voipline\n",__FUNCTION__);
			EnableSPKUnit(duaUnitSpk,0);
			connect_wbhf_voipline(init_rtp_session_config.voip_line_id);
			printf("%s - connected wbhf_voipline\n",__FUNCTION__);*/
		}
	}
	return;
}

int dua_UnitSet_CodecSpecificConfig()
{
       int nRes = 0;
#ifdef UE_G6E
	if((!strcasecmp(init_rtp_session_config.codec.rx_list[0].CodecStr,STR_CODEC_G726_16)) ||
			(!strcasecmp(init_rtp_session_config.codec.rx_list[0].CodecStr,STR_CODEC_G726_24)) ||
			(!strcasecmp(init_rtp_session_config.codec.rx_list[0].CodecStr,STR_CODEC_G726_32)) ||
			(!strcasecmp(init_rtp_session_config.codec.rx_list[0].CodecStr,STR_CODEC_G726_40))){
		nRes = mediaext_g726_packing_mode(DUA_UID(UT_X_VOIP,init_rtp_session_config.voip_line_id), 0);
		if(nRes < 0){
			printf("\nSetting g726 packing mode Failed\n");
		} else {
			printf("\nSetting g726 packing mode Success\n");
		}
	}
#endif
#ifdef UE_AMRWE
	if(!strcasecmp(init_rtp_session_config.codec.rx_list[0].CodecStr, STR_CODEC_AMRWB)){
		nRes = mediaext_setamr_wb_codecParams(DUA_UID(UT_X_VOIP,init_rtp_session_config.voip_line_id),&init_rtp_session_config.codec.vbrCodecParam.amrwb);
		if(nRes < 0){			
					printf("\nSetting AMR-WB  params  Failed\n");
		} else {
					printf("\nSetting AMR-WB params  Success\n");
		}

	}
#endif
#ifdef UE_OPUSE
	if(!strcasecmp(init_rtp_session_config.codec.rx_list[0].CodecStr, STR_CODEC_OPUS)){
		nRes = mediaext_setOPUSParams(DUA_UID(UT_X_VOIP,init_rtp_session_config.voip_line_id),&opus_codec_param);
		if(nRes < 0){
					printf("\nmediaext_setOPUSParams Failed\n");
		} else {
					printf("\nmediaext_setOPUSParams Success\n");
		}
	}
#endif
	// Send SID update
	nRes = mediaext_sendsidupdate(DUA_UID(UT_X_VOIP,init_rtp_session_config.voip_line_id),init_rtp_session_config.sid_update,
			init_rtp_session_config.codec.rx_list[0].rx_pt,init_rtp_session_config.codec.rx_list[0].CodecStr);
	if(nRes < 0){
				printf("\nSid update Failed\n");
	} else {
				printf("\nSid update Success\n");
	}
	return nRes;
}

int app_rtp_init(void)
{
	unsigned int flag_register = 0;
	hdjfslfj;
	if (rtp_init() != RTP_APP_SUCCESS){
		printf (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>rtp_init() error!!\n");
		return 0;
	}
	/***********************************rtp_app_registe***********************************/
	flag_register |= RTP_EVENT_RX_RFC2833;
	flag_register |= RTP_EVENT_RX_FIRST_RTP_PACKET;
	flag_register |= RTP_EVENT_TX_FIRST_RTP_PACKET;
	flag_register |= RTP_EVENT_RX_MARKER_BIT_SET;
	flag_register |= RTP_EVENT_RX_SSRC_CHANGE;
	flag_register |= RTP_EVENT_RX_PAYLOAD_CHANGE;
	if (rtp_app_register(RTP_APP_PROCESS_MODE , flag_register , RTP_APP_SIP , voip_rtpcallback) != RTP_APP_SUCCESS){
		printf (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>rtp_app_register() error!!\n");
		return 0;
	}
	/*************************************************************************************/
	Init_rtp_session_config ();
	/*************************************************************************************/
	return 1;
}

int Start_stream (char * remote_ip,int local_port,int remote_port){
	rtp_config rtpCfg;
	rtcp_config rtcpCfg;
	struct sockaddr_in *sockaddr_ipv4_local = NULL; 
	struct sockaddr_in *sockaddr_ipv4_remote = NULL;
	struct sockaddr_in sock_buff;
	int ret = 0;
	printf("##############Start_stream##############\n");
	/***********************************rtp_cfg_ipsock***********************************/
	memset(&rtpCfg, 0, sizeof(rtp_config));
	memset(&rtcpCfg,0,sizeof(rtcp_config));
	sockaddr_ipv4_local = (struct sockaddr_in *)&(rtpCfg.rtp_local_addr);
	sockaddr_ipv4_remote = (struct sockaddr_in *)&(rtpCfg.rtp_remote_addr);
	{
		sockaddr_ipv4_local->sin_family = AF_INET;
		sockaddr_ipv4_local->sin_port = htons(local_port);
		sockaddr_ipv4_local->sin_addr.s_addr  =  htonl(INADDR_ANY);
		sockaddr_ipv4_remote->sin_family = AF_INET;
		sockaddr_ipv4_remote->sin_port = htons(remote_port);
		inet_aton(remote_ip, &sockaddr_ipv4_remote->sin_addr);
	}
	/***********************************rtp_cfg_rtcp_ipsock***********************************/
	sockaddr_ipv4_local = (struct sockaddr_in *)&(rtcpCfg.rtcp_local_addr);
	sockaddr_ipv4_remote = (struct sockaddr_in *)&(rtcpCfg.rtcp_remote_addr);
	{
		sockaddr_ipv4_local->sin_family = AF_INET;
		sockaddr_ipv4_local->sin_port = htons(local_port+1);
		sockaddr_ipv4_local->sin_addr.s_addr  =  htonl(INADDR_ANY);
		sockaddr_ipv4_remote->sin_family = AF_INET;
		sockaddr_ipv4_remote->sin_port = htons(remote_port+1);
		inet_aton(remote_ip, &sockaddr_ipv4_remote->sin_addr);
	}
	rtcpCfg.rtcp_config.rbType = RTCP_XR_DISABLE;
	rtcpCfg.rtcp_config.opts |= RTCP_MODE_RX;
	rtcpCfg.rtcp_config.opts |= RTCP_MODE_TX;
	rtcpCfg.rtcp_config.opts |= RTCP_KERNEL_MODE;
	rtcpCfg.rtcp_config.rtcp_interval=5;

	memset ( rtcpCfg.rtcp_config.sdesItem, 0, MAX_SDES_ITEMS * MAX_SDES_VAL_LEN);

	rtpCfg.rtp_config.opts |=(RTP_SESSION_OPT_RTCP_ON);
	init_rtp_session_config.opts |= RTP_SESSION_OPT_RTCP_ON;
	
	memcpy(&sock_buff,(struct sockaddr_in *)&rtpCfg.rtp_local_addr,sizeof(struct sockaddr_in));
	printf ("RTP LOCAL IP is %s port %d\n",inet_ntoa(sock_buff.sin_addr),sock_buff.sin_port);
	memset (&sock_buff,0,sizeof(sock_buff));
	memcpy(&sock_buff,(struct sockaddr_in *)&rtpCfg.rtp_remote_addr,sizeof(struct sockaddr_in));
	printf ("RTP ROMOTE IP is %s port %d\n",inet_ntoa(sock_buff.sin_addr),sock_buff.sin_port);
	memset (&sock_buff,0,sizeof(sock_buff));
	memcpy(&sock_buff,(struct sockaddr_in *)&rtcpCfg.rtcp_local_addr,sizeof(struct sockaddr_in));
	printf ("RTCP LOCAL IP is %s port %d\n",inet_ntoa(sock_buff.sin_addr),sock_buff.sin_port);
	memset (&sock_buff,0,sizeof(sock_buff));
	memcpy(&sock_buff,(struct sockaddr_in *)&rtcpCfg.rtcp_remote_addr,sizeof(struct sockaddr_in));
	printf ("RTCP ROMOTE IP is %s port %d\n",inet_ntoa(sock_buff.sin_addr),sock_buff.sin_port);
	memset (&sock_buff,0,sizeof(sock_buff));
	/***********************************rtp_app_registe***********************************/
	memcpy ((char *)&(rtpCfg.rtp_config),(char *)&(init_rtp_session_config),sizeof(rtp_session_config));
	/***********************************start_rtcp***********************************/
	ret = rtp_session_start(init_rtp_session_config.session_id,&rtpCfg,&rtcpCfg);
	if( ret != RTP_APP_SUCCESS){
		printf("##############rtp_session_start failed for session id : %d reason : %d##############\n",init_rtp_session_config.session_id,ret);
		return 0;
	}
	else{
		printf("##############rtp_session_start success for session id : %d reason : %d##############\n",init_rtp_session_config.session_id,ret);
		if(init_rtp_session_config.audio_mode != RTP_MODE_INACTIVE){			
			dua_UnitSet_CodecSpecificConfig();
		}
	}
	return 1;
}

void Stop_stream(void){
	rtp_session_stop(init_rtp_session_config.session_id);
}

void Init_cmrtp (void){
	app_rtp_init ();
}


