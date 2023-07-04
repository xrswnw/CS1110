#ifndef _ANYID_CS1110_SOUND_
#define _ANYID_CS1110_SOUND_

#include "AnyID_CS1110_Config.h"
#include "AnyID_CS1110_Sound_HL.h"
#include "AnyID_CS1110_SysTick_HL.h"

#define SOUND_STAT_IDLE              0x00
#define SOUND_STAT_RCV               0x01
#define SOUND_STAT_OVR               0x02
#define SOUND_STAT_TX                0x04
#define SOUND_STAT_WAIT              0x08
#define SOUND_STAT_END               0x10

#define SOUND_FLAG_IDLE              0x00
#define SOUND_FLAG_HRAD              0x01
#define SOUND_FLAG_ADD               0x02
#define SOUND_FLAG_LEN               0x03
#define SOUND_FLAG_DATA              0x04
#define SOUND_FLAG_FAIL              0x04

#define SOUND_FRAME_HEAD                0x7E
#define SOUND_FRAME_EOF                 0xEF
#define SOUND_FRAME_ADD                 0xFF
#define SOUND_FRAME_CMD_APPOINT_NUM     0x0F
#define SOUND_FRAME_CMD_VOL_HIGH        0x04
#define SOUND_FRAME_CMD_VOL_LOW         0x05
#define SOUND_FRAME_CMD_ROUND_STAR      0x11
#define SOUND_FRAME_CMD_ROUND_STOP      0x16
#define SOUND_FRAME_CMD_GET_VOL         0x43
#define SOUND_FRAME_CMD_GET_NUM         0x49
#define SOUND_FRAME_CMD_APPOINT_FIDLE   0x0F
#define SOUND_BUF_LEN		        20
#define SOUND_BUF_MIN_LEN		0x05

/*
#define SOUND_VOC_PUT_TAG               0x0001
#define SOUND_VOC_NO_BIND               0x0002
#define SOUND_VOC_TAG_CLASH             0x0003
#define SOUND_VOC_OFF_LINE              0x0004
#define SOUND_VOC_PUT_OVER              0xFFFF
*/

#define SOUND_VOC_PUT_TAG               0x0101
#define SOUND_VOC_NO_BIND               0x0102
#define SOUND_VOC_TAG_CLASH             0x0103
#define SOUND_VOC_OFF_LINE              0x0104


#define SOUND_TEST_FLAG_DISABLE         0x0000
#define SOUND_TEST_FLAG_ENABLE          0x0001

#define SOUND_VOC_STOP_TIM              2000
#define Sound_IsRcvFrame(rcvFrame)               ((rcvFrame).state == SOUND_STAT_RCV || (rcvFrame).state == SOUND_STAT_OVR)
#define Sound_ResetFrame(rcvFrame)               do{(rcvFrame).state = SOUND_STAT_IDLE ; (rcvFrame).repeat =0;}while(0)
                


typedef struct soundTxBuffer{
    u8 cmd;
    u8 len;
    u8 buffer[SOUND_BUF_LEN];
    u16 data;
}SOUND_TX_BUF;


typedef struct soundRxBuffer{
    u8 cmd;
    u8 len;
    u8 buffer[SOUND_BUF_LEN];
    u16 data;
}SOUND_RX_BUF;


typedef struct soundInfo{
    u8 state;
    u8 flag;
    u8 repeat;
    u8 index;
    u16 testFlag;
    u32 tick; 
    SOUND_TX_BUF txBuf;
    SOUND_RX_BUF rxBuf;
}SOUND_INFO;
extern SOUND_INFO g_sSoundInfo ;

void Sound_Init();
void Sound_ReceiveFrame(u8 byte, SOUND_INFO *Info);
u8 Sound_FormaFrame(u8 cmd,u8 feedback, u16 dat, u8 *pFrame);
u8 Sounde_Chk(u8 *pFrame);
void Sound_Sum(u8 *pFrame, u8 len);
void Sound_TransmitCmd(u8 cmd, u8 feedback,u16 data);


#endif
