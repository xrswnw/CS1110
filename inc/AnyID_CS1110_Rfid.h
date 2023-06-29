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

#define RFID_FLAG_IDLE              0x00
#define RFID_FLAG_HRAD              0x01
#define RFID_FLAG_ADD               0x02
#define RFID_FLAG_LEN               0x03
#define RFID_FLAG_DATA              0x04
#define RFID_FLAG_FAIL              0x04

#define RFID_GET_UID                0x30
#define RFID_FRAME_LEN              30

#define RFID_QEQUST_TIM             100
#define RFID_QEQUST_TICK             5
typedef struct rfidInfo{
    u8 state;
    u8 repat;
    u16 len;
    u8 buffer[RFID_FRAME_LEN];
    u32 tick;
    UART_RCVFRAME rfidRev;
}RFID_INFO;
extern RFID_INFO g_sRfidInfo;


void Rfid_Init();
u8 Rfid_Format_GetUid(u8 cmd,  RFID_INFO *pOpResult);
void Rfid_TransmitCmd(u32 tick);
void Rfid_ReceiveFrame(u8 byte, UART_RCVFRAME *pRcvFrame);
#endif
