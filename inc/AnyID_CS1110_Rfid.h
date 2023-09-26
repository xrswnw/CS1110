#ifndef _ANYID_CS1110_RFID_
#define _ANYID_CS1110_RFID_

#include "AnyID_CS1110_Config.h"
#include "AnyID_CS1110_Rfid_HL.h"
#include "AnyID_CS1110_SysTick_HL.h"
#include "AnyID_Uart_Receive.h"


#define RFID_STAT_IDLE              0x00
#define RFID_STAT_RCV               0x01
#define RFID_STAT_OVR               0x02
#define RFID_STAT_TX                0x04
#define RFID_STAT_WAIT              0x08
#define RFID_STAT_END               0x10
#define RFID_STAT_DELAY_TIME		0x20

#define RFID_FLAG_IDLE              0x00
#define RFID_FLAG_HRAD              0x01
#define RFID_FLAG_ADD               0x02
#define RFID_FLAG_LEN               0x03
#define RFID_FLAG_DATA              0x04
#define RFID_FLAG_FAIL              0x04


#define RFID_VERSION_SIZE			50
#define RFID_MODE_GET_UID		    0
#define RFID_MODE_GET_VERSION		1

#define RFID_ONE_TAG_TIME			3
#define RFID_ONE_MORE_TIME			4


#define RFID_GET_VERSION			0xF7
#define RFID_GET_UID                0x30
#define RFID_FRAME_LEN              30

#define RFID_TAG_NULL                    0x00
#define RFID_TAG_IN                      0x01
#define RFID_TAG_KEEP                    0x02
#define RFID_TAG_OUT                     0x04
#define RFID_TAG_FAIL                    0x08

#define RFID_LINK_OK					0
#define RFID_LINK_FAIL					1

#define RFID_DTU_FLAG					1
#define RFID_QEQUST_DELAY_TIM       10
#define RFID_QEQUST_TIM             100
#define RFID_QEQUST_TICK             5

#define RFID_OP_DLY_TIM				20

#define Rfid_StartOpDelay(t) 			do{g_sRfidInfo.state = RFID_STAT_DELAY_TIME; g_sRfidInfo.tick = (t); }while(0)


#define Rfid_ResetFrame(rcvFrame)     do{(rcvFrame)->state = UART_STAT_IDLE; (rcvFrame)->length = 0; (rcvFrame)->repat = 0;(rcvFrame)->index = 0;memset(&g_sRfidRcvTempFrame, 0, sizeof(UART_RCVFRAME));memcpy(&g_sRfidRcvTempFrame, &g_sRfidInfo.rfidRev, sizeof(UART_RCVFRAME));}while(0)
typedef struct rfidInfo{
	u8 mode;
    u8 state;
    u8 repat;
	u8 flag;
    u16 len;
    u8 buffer[RFID_FRAME_LEN];
	u32 delayTick;
    u32 tick;
	u8 verSion[RFID_VERSION_SIZE];
    UART_RCVFRAME rfidRev;
}RFID_INFO;
extern RFID_INFO g_sRfidInfo;
extern u32 g_nRfidRomNum;

void Rfid_Init();
u8 Rfid_Format_GetUid(u8 cmd,  RFID_INFO *pOpResult);
u16 Rfid_FormatDtuFrame(u8 cmd, u16 len, u8 *pParam, RFID_INFO *pOpResult);
void Rfid_TransmitCmd(u32 tick);
void Rfid_ReceiveFrame(u8 byte, UART_RCVFRAME *pRcvFrame);
#endif
