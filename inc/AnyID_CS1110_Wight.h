#ifndef _ANYID_CS1110_WIGHT_H
#define _ANYID_CS1110_WIGHT_H

#include "AnyID_CS1110_Config.h"
#include "AnyID_CS1110_Wight_HL.h"

#define GPB_REG_ADDR_WGT		0x0001
#define GPB_REG_ADDR_ZERO		0x0002
#define GPB_REG_ADDR_SPEED		0x0009
#define GPB_REG_ADDR_RTO		0x000B
#define GPB_REG_ADDR_SET_ADDR		0x000F


#define GPB_STAT_IDLE 	                0x00000000
#define GPB_STAT_RCV 		        0x00000001
#define GPB_STAT_OVR 		        0x00000004
#define GPB_STAT_TX 		        0x00000008
#define GPB_STAT_TX_IDLE                0x00000010
#define GPB_STAT_WAIT                   0x00000020

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

#define GPB_WITGH_MASK_VALUE            0x10000000
#define GPB_WITGH_LOW_MASK_VALUE        10
#define GPB_WITGH_FLAG_PLUS             0
#define GPB_WITGH_FLAG_MINUS            0x10000000
#define GPB_WIGHT_VALUE_POS             0x03
#define GPB_BUFFER_MAX_LEN             (256 + 32) 

#define GPB_WOEK_NORMAL                 0x00
#define GPB_WORK_SET_ZERO               0x01

#define GPB_STAT_OPEN_SAMPLE            0x01
#define GPB_STAT_STOP_SAMPLE            0x02

#define GPB_FLAG_NORMAL                 0
#define GPB_FLAG_FAIL                   0x01
#define GPB_SAMPLE_NUM                  16
typedef struct wightInfo{
    u32 flag;
    u32 sum;
    u32 avg;
    u32 index;
    u32 buffer[GPB_SAMPLE_NUM];
    u32 alarmTick;
}WIGHT_INFO;


extern WIGHT_INFO g_sWightTempInfo;
extern WIGHT_INFO g_sWigthInfo;

#define GPB_STAT_SAMPLE_NUM             5
#define Gpb_IsRcvFrame(rcvFrame)               ((rcvFrame).state == GPB_STAT_RCV || (rcvFrame).state == GPB_STAT_OVR)
#define Gpb_ResetFrame(rcvFrame)               do{(rcvFrame).state = GPB_STAT_IDLE; rcvFrame.repeat=0;}while(0)

#define Gpb_GetValue(p)                 ({\
                                             u32 value;\
                                             value = (*(p + GPB_WIGHT_VALUE_POS + 0) << 24) | (*(p + GPB_WIGHT_VALUE_POS + 1) << 16) | (*(p + GPB_WIGHT_VALUE_POS + 2) << 8) | (*(p + GPB_WIGHT_VALUE_POS + 3) << 0);\
                                             (value);\
                                         })\

#define Gpb_Get_Dish_WightValue()       ({\
                                            u32 value;\
                                            value = g_sGpbInfo.wightValue - g_sGpbInfo.wightTemp;\
                                              (value & 0x80000000? 0 : value);\
                                        })\
                                           
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
    
    u8 flag;
    u8 repeat;
    u8 index;
    u8 num;
    u8 mode;
    u32 state;
    u32 witghSmple;
    u32 wightValue;
    u32 wightTemp;
    u32 maskValue;
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
void Witgh_CalAvg(WIGHT_INFO *pInfo, u32 value);

#endif