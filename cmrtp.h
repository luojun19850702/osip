#ifndef CMRTP_H
#define CMRTP_H

#define CODEC_UNKNOWN                           -1
#define CODEC_G711_MLAW                         0   ///< PCM - Mu law
#define CODEC_G726_32_STATIC                    2   ///< 32 kbps ADPCM
#define CODEC_GSM  								3   ///< GSM 

#define CODEC_G7231                             4   ///< Dual rate 5.3/6.3 kbps

#define CODEC_G711_ALAW                         8   ///< PCM - A law
#define CODEC_G722                              9   ///< WIDE BAND - 16kHz, rate 64 kbps
#define CODEC_COMFORT_NOISE                     13  ///< Comfort Noise
#define CODEC_G729                              18  ///< CS-ACELP (normal/low-complexity) G.729A, G.729B, G.729AB -  check what Cisco does 

#define	CODEC_G726_16							111
#define	CODEC_G726_24							112
#define CODEC_G726_40							113
#define CODEC_G726_32_DYNAMIC                   123

#define CODEC_DYNAMIC_iLBC_97                    97
#define CODEC_DYNAMIC_iLBC_98                    98
#define CODEC_DYNAMIC_iLBC_99                    99


#define	CODEC_AMRWB_7k							102 		// AMR-WB-7k Start
#define	CODEC_AMRWB_9k							103
#define	CODEC_AMRWB_12							104
#define	CODEC_AMRWB_14							105
#define	CODEC_AMRWB_16							106
#define	CODEC_AMRWB_18							107
#define	CODEC_AMRWB_20							108
#define	CODEC_AMRWB_23							109
#define	CODEC_AMRWB_24							110


#define	CODEC_AMRNB_4K75						114 		// AMR-NB - 4.75 kbit/sec START
#define	CODEC_AMRNB_5K15						115        // 5.15 kbit/sec
#define	CODEC_AMRNB_5K9							116        // 5.9  kbit/sec
#define	CODEC_AMRNB_6K7							117        // 6.70 kbit/sec
#define	CODEC_AMRNB_7K4							118	      // 7.4  kbit/sec
#define	CODEC_AMRNB_7K95						119	      // 7.95 kbit/sec
#define	CODEC_AMRNB_10K2						120	      // 10.2 kbit/sec
#define	CODEC_AMRNB_12K2						121    //AMR- 12.2 kbit/sec  End

#define CODEC_L16_256                           122

#define CODEC_OPUS                              125

#define CODEC_G711_MLAW_DYNAMIC                  96
#define CODEC_G711_ALAW_DYNAMIC                 100
#define CODEC_G722_DYNAMIC                      126
#define CODEC_G729_DYNAMIC                      127
#define INCALL                                   1


#define DEFAULT_PTIME_G711U 	20
#define DEFAULT_PTIME_G711A		20
#define DEFAULT_PTIME_G729 		20
#define DEFAULT_PTIME_G722 		20
#define DEFAULT_PTIME_G726_16 	20
#define DEFAULT_PTIME_G726_24 	20
#define DEFAULT_PTIME_G726_32 	20
#define DEFAULT_PTIME_G726_40 	20
#define DEFAULT_PTIME_ILBC_15K 	20
#define DEFAULT_PTIME_ILBC_13K 	30
#define DEFAULT_PTIME_AMRWB		20
#define DEFAULT_PTIME_L16_256   20
#define DEFAULT_PTIME_OPUS		20
#define DEFAULT_PTIME_G723		30

// RFC 3555
#define	STR_CODEC_PCMU				"pcmu/8000\0"
//#define	STR_CODEC_G721			"G721/8000"
#define	STR_CODEC_G721				"G726-32/8000\0"
#define	STR_CODEC_GSM				"GSM/8000\0"
#define	STR_CODEC_G722				"G722/8000\0"
#define	STR_CODEC_G723				"G723/8000\0"
#define	STR_CODEC_PCMA				"pcma/8000\0"
#define	STR_CODEC_G729				"G729/8000\0"
#define	STR_CODEC_iLBC				"iLBC/8000\0"


#define	STR_CODEC_AMRWB				"AMR-WB/16000\0"
#define STR_CODEC_L16_256           "L16/16000\0"
#define	STR_CODEC_G726_16			"G726-16/8000\0"
#define	STR_CODEC_G726_24			"G726-24/8000\0"
#define STR_CODEC_G726_32           "G726-32/8000\0"
#define	STR_CODEC_G726_40			"G726-40/8000\0"
#define STR_CODEC_OPUS     		    "OPUS/48000\0"

extern rtp_session_config init_rtp_session_config;

extern void Init_cmrtp (void);
extern int Start_stream (char * remote_ip,int local_port,int remote_port);
extern void Stop_stream(void);
#endif
