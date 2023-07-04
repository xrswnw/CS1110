#include "AnyID_CS1110_Sound.h"
#include <math.h>
SOUND_INFO g_sSoundInfo = {0};

void Sound_Init()
{
    Sound_InitInterface(SOUND_BAUDRARE);
    Sound_ConfigInt();
    Sound_EnableInt(ENABLE,ENABLE);
    g_sSoundInfo.state = SOUND_STAT_IDLE;
}

void Sound_ReceiveFrame(u8 byte, SOUND_INFO *Info)
{

    switch(Info->flag)
    {
        case SOUND_FLAG_IDLE:
            if(byte == SOUND_FRAME_HEAD)
            {
                Info->rxBuf.buffer[0] = byte;
                Info->flag = SOUND_FLAG_HRAD;
            }
            break;
        case SOUND_FLAG_HRAD:
            if(byte == SOUND_FRAME_ADD)
            {
                Info->rxBuf.buffer[1] = byte;
                Info->flag = SOUND_FLAG_LEN;
            }
            else
            {
                Info->flag = SOUND_FLAG_IDLE;
            }
            break;
        case SOUND_FLAG_LEN:
            if(SOUND_BUF_MIN_LEN <= byte)
            {
                Info->flag = SOUND_FLAG_DATA;
                Info->rxBuf.len  = byte ;
                Info->rxBuf.buffer[2] = byte;
                Info->index = 3;
            }
            else
            {
                Info->flag = SOUND_FLAG_IDLE;
            }
            break;
        case SOUND_FLAG_DATA:
            Info->rxBuf.buffer[Info->index++] = byte;
            
            if(Info->index > (Info->rxBuf.len + 3))
            {
                Info->flag = SOUND_FLAG_IDLE;
                Info->state = SOUND_STAT_RCV;
            }
            else
            {
                Info->state = SOUND_STAT_IDLE;
            }

            break;
    }
}

u8 Sound_FormaFrame(u8 cmd,u8 feedback, u16 dat, u8 *pFrame)
{
	u8 pos = 0;
	pFrame[pos++] = SOUND_FRAME_HEAD;
	pFrame[pos++] = SOUND_FRAME_ADD;
	pFrame[pos++] = 0x06;
	pFrame[pos++] = cmd;
	pFrame[pos++] = feedback;
	pFrame[pos++] = (dat >> 8) & 0xFF;
	pFrame[pos++] = (dat >> 0) & 0xFF;
	Sound_Sum(&pFrame[1],6); 
        pos += 3;
	return pos;
}

void Sound_Sum(u8 *pFrame, u8 len)
{
     u16 sum = 0;
     u8 i;
     for(i = 0; i < len; i++) 
     {
            sum = sum + pFrame[i];
     }
    sum = 0 - sum;
    *(pFrame+i)   = (sum >>8) & 0xFF;
    *(pFrame+i+1) = (sum >>0) & 0xFF;
    *(pFrame+i+2) = 0xEF;
}

u8 Sounde_Chk(u8 *pFrame)
{       
    u8 bOk = FALSE; 
    u8 i = 0;
    u16 sumS = 0;
    u16 sumG = 0;
    for(i=1; i <= (*(pFrame+2)); i++)
     {
        sumS = sumS + pFrame[i] ;
     }
     sumG = ((u16)((*(pFrame+i))<<8)) | (*(pFrame+i+1));
     sumS = sumS + sumG;
     if(!sumS)
     {
       bOk = TRUE;
     }

     return bOk;
}
void Sound_TransmitCmd(u8 cmd, u8 feedback,u16 data)
{
    g_sSoundInfo.txBuf.len = Sound_FormaFrame(cmd, feedback, data,  g_sSoundInfo.txBuf.buffer);
    Sound_WriteBuffer(g_sSoundInfo.txBuf.buffer, g_sSoundInfo.txBuf.len);
}
