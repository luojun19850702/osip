#ifdef VOIP_CNTRL_ENH
#include <voice.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <rtp.h>
#include <unistd.h>
char getDigit()
{
        static char digits[]={'0', '1', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', '*', '#'};
        static int index = 0;
        return(digits[index++]);
        if(index >=14)
           index = 0;
        
}

void dtmf_intract()
{
        char continuous = 0;
        int ret= 0;
        rtp_dtmf_event dtmf;
	memset(&dtmf, 0,  sizeof(rtp_dtmf_event));
	printf("Digit (0-9): ");
	scanf("%c", (char *)&dtmf.event);
	dtmf.status = RTP_DTMF_STATUS_START;
	printf("Enter DTMF volume[Default: 20 - range - 0 - 63] : \n");
	scanf("%d", &dtmf.volume);
	printf("Enter DTMF Duration[Default: 20] : \n");
	scanf("%d",&dtmf.duration);
	printf("Enter DTMF EvtDuration[Default: 160] : \n");
	scanf("%d",&dtmf.EvtDuration);
	printf("Enter DTMF MaxEvtDuration[Range: 1 - 1048575(0xFFFFF)] : \n");
	scanf("%d",&dtmf.MaxEvtDuration);
	printf("Do you want to send digits continuously?[1 - Yes -- 0 - N0] \n");
	scanf("%d",(int *)&continuous);
	printf("Enter value for Opts[1 - set end bit in all evt pkts, -- 0 - set end bit only in end pkt]\n");
	scanf("%d",&dtmf.Opts);
	if(!continuous)
	{
		ret = rtp_session_dtmf_send(0, &dtmf);
		if(ret < 0 ){
			printf(" rtp_session_dtmf_send Fail - Invalid max event duration\n");
		}

	}
	else
	{
		while(1)
		{
			usleep(500000);
			dtmf.event = getDigit();
			printf("Sending DTMF digit %d\n",dtmf.event);
			ret = rtp_session_dtmf_send(0, &dtmf);
			if(ret > 0)
			{
				printf(" rtp_session_dtmf_send Pass for digit %d - ret(%d)\n",dtmf.event,ret);
			}
		}
	}

}
#endif
