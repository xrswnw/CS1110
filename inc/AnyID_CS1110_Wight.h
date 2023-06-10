#ifndef _ANYID_CS1110_WIGHT_H
#define _ANYID_CS1110_WIGHT_H

#include "AnyID_CS1110_Config.h"
#include "AnyID_CS1110_Wight_HL.h"

#define GPB_REG_ADDR_WGT		0x0001
#define GPB_REG_ADDR_ZERO		0x0002
#define GPB_REG_ADDR_SPEED		0x0009
#define GPB_REG_ADDR_RTO		0x000B
#define GPB_REG_ADDR_SET_ADDR		0x000F


#define GPB_STAT_IDLE 	                0x00
#define GPB_STAT_RCV 		        0x01
#define GPB_STAT_OVR 		        0x04
#define GPB_STAT_TX 		        0x08
#define GPB_STAT_TX_IDLE                0x10
#define GPB_STAT_WAIT                   0x20

#define GPB_FLAG_IDLE 	                0x00
#define GPB_FLAG_CMD 		        0x01
#define GPB_FLAG_REG 		        0x04
#define GPB_FLAG_OVR		        0x08

#define GPB_MODBUS_READ_REG             0x03
#define GPB_MODBUS_SET_REG              0x10

#define GPB_BUF_MAX_LEN	                0x20
#define GPB_BUF_MIN_LEN	                0x09

#define GPB_WIGHT_VALUE_STABLE  	0x01
#define GPB_WIGHT_VALUE_FAIL      	0x00
#define GPBWIGHT_VALUE_NUM		3


#define GPB_WIGHT_VALUE_POS             0x03
#define GPB_BUFFER_MAX_LEN             (256 + 32) 

#define GPB_WOEK_NORMAL                 0x00
#define GPB_WORK_SET_ZERO               0x01

#define GPB_STAT_OPEN_SAMPLE            0x01
#define GPB_STAT_STOP_SAMPLE            0x02


#define GPB_STAT_SAMPLE_NUM             3
#define Gpb_IsRcvFrame(rcvFrame)               ((rcvFrame).state == GPB_STAT_RCV || (rcvFrame).state == GPB_STAT_OVR)
#define Gpb_ResetFrame(rcvFrame)               do{(rcvFrame).state = GPB_STAT_IDLE; rcvFrame.repeat=0;}while(0)

typedef struct wihgtTxBuf{
    u8 cmd;
    u16 regAdd;
    u16 regNum;
    u16 data0;
    u16 data1;
    u32 len;
    u8 buffer[GPB_BUF_MAX_LEN];
    u32 crc16;
}GPB_TX_BUF;

typedef struct wihgtRxBuf{
    u8 cmd;
    u32 len;
    u8 hg;
    u8 point;
    u32 wight;
    u16 regAdd;
    u8 buffer[GPB_BUF_MAX_LEN];
    u32 crc16;
}GPB_RX_BUF;

typedef struct wihgtInfo{
    u8 state;
    u8 flag;
    u8 repeat;
    u8 index;
    u8 num;
    u8 mode;
    u8 witghSmple;
    u32 wightValue;
    u32 wightTemp;
    u32 tick; 
    GPB_TX_BUF txBuf;
    GPB_RX_BUF rxBuf;
}GPB_INFO;
extern GPB_INFO g_sGpbInfo ;
extern GPB_RX_BUF g_sGpbTempRcv;

void GPB_Init();
void GPB_TransmitCmd(GPB_TX_BUF *pBuffer);
void GPB_ChgValue(u32 value, char *buf);
void GPB_Adjust(GPB_TX_BUF *pBuffer);
void GPB_RtoChg(GPB_TX_BUF *pBuffer);
void GPB_Filt_Chg();
BOOL GPB_CheckFrame(GPB_RX_BUF *pRcvFrame);

u8 GPB_GetValue(GPB_INFO *pBuffer);
u8 GPB_FormatFrame(u8 cmd,u16 regAdd,u16 regNum,u8 *pFrame);
u16 GPB_GetCrc16(u8 *pBuffer, u8 len);
u32 GPB_GetWightValue(u32 sampleValue);


#endif