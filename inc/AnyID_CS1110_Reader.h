#ifndef _ANYID_CS1110_READER_H
#define _ANYID_CS1110_READER_H

#include "AnyID_CS1110_Config.h"
#include "AnyID_ST25R391x.h"
#include "AnyID_CS1110_DishLcm.h"
#include "AnyID_CS1110_Uart.h"
#include "AnyID_ST25R391x_ISO15693.h"
#include "AnyID_CS1110_FRam.h"
#include  "stdlib.h"
#include "AnyID_CS1110_Key.h"
#include "AnyID_CS1110_Sound.h"
#include "AnyID_CS1110_Wight.h"
#include "AnyID_CS1110_SysTick_HL.h"
#include "AnyID_FRam.h"
#include "AnyID_CS1110_RTC.h"
#include "AnyID_Uart_Receive.h"
#include "AnyID_CS1110_FlashM25.h"
#include "AnyID_CS1110_Rfid.h"
#include "AnyID_Cs1110_WDG_HL.h"

#define READER_RSP_OFFLINE_DATA_LEN             28
#define READER_VERSION_SIZE               50
#define READER_RFID_READ_UID              0x01
#define READER_RFID_READ_BLOCK            0x02





#define READER_STAT_IDLE                   0x00000000
#define READER_STAT_DTU                    0x00000001
#define READER_STAT_CHK_UPDATA             0x00000002
#define READER_STAT_PUT_TAG                0x00000004
#define READER_STAT_CLASH_TAG              0x00000008
#define READER_STAT_UARTTX                 0x00000010
#define READER_STAT_BIND_TAG               0x00000020
#define READER_STAT_UART_WAIT              0x00000040
#define READER_STAT_UARTRX                 0x00000080
#define READER_STAT_RFID_KEEP              0x00000100
#define READER_STAT_MODE_NORMAL            0x00000200
#define READER_STAT_READER_MARK_OK         0x00000400
#define READER_STAT_RFID_FAIL              0x00000800
#define READER_STAT_KEY                    0x00001000
#define READER_STAT_LCM_FRESH              0x00002000
#define READER_STAT_CHK_LINK               0x00004000
#define READER_STAT_UART_TX_WAIT           0x00008000
#define READER_STAT_WIGHT_ZERO_LOADING     0x00010000




#define RFID_ANT_OPEN                     0x00
#define RFID_ANT_CLOSE                    0x01

#define READER_TOTAL_MODE_AIR             0x00
#define READER_TOTAL_MODE_AFTER           0x01


#define READER_UI_MODE_WHITE             0x00
#define READER_UI_MODE_BLACK             0x0A

#define READER_RSPFRAME_FLAG_OK          0x00
#define READER_RSPFRAME_FLAG_FAIL        0x01
#define READER_OPTAG_RESPONSE_NOERR      0x00
#define READER_OPTAG_RESPONSE_TAGERR     0x01
#define READER_OPTAG_RESPONSE_CRCERR     0x02
#define READER_OPTAG_RESPONSE_NORSP      0x03
#define READER_OPTAG_RESPONSE_PARERR     0x04

#define RFID_TAG_NULL                     0x00
#define RFID_TAG_IN                      0x01
#define RFID_TAG_KEEP                    0x02
#define RFID_TAG_OUT                     0x04
#define RFID_TAG_FAIL                    0x08

#define READER_RFID_UID_POST             19

#define READER_MODE_NORMAL               0x00
#define READER_MODE_TEST                 0x01
#define READER_MODE_STOP                 0x02

#define READER_FRAME_BROADCAST_ADDR         0xFFFF

//命令
#define READER_CMD_INVENTORY                0x00
#define READER_CMD_RESET                    0x04
#define READER_CMD_RF_CTRL                  0xF0
#define READER_CMD_GET_VERSION              0xF7
#define READER_CMD_GET_CPUID                0xF8
#define READER_CMD_CHG_IP                   0xFF
#define READER_CMD_DRU_CMD                  0xFE

#define READER_CMD_SET_CFG                  0x20
#define READER_CMD_GET_CFG                  0x21
#define READER_CMD_GET_PERSON_INFO          0x22
#define READER_CMD_GET_MEAL_INFO            0x23
#define READER_CMD_GET_HEART                0x25

#define READER_CMD_DRU_GPB                  0x30
#define READER_CMD_DRU_SOUND                0x31
#define READER_CMD_DRU_LED                  0x32
#define READER_CMD_DRU_LCD                  0x33
#define READER_CMD_DRU_GET_KEY              0x34


#define READER_DTU_FLAG_VOICD                   0x01
#define READER_DTU_FLAG_LCM                     0x02
#define READER_DTU_FLAG_GPB                     0x04

#define READER_STATUS_ENTOURAGR                0
#define READER_STATUS_MASTER                0x01
#define READER_RES_OK                       0x00

#define READER_VERSION_LEN                  8
#define READER_SET_CFG                      8

#define READER_MEAL_RTC_POS                 9

#define READER_MEAL_NAME                    13
#define READER_MEAL_NAME_LEN                13

#define READER_MEAL_RTC_LEN                 10

#define READER_PERSON_UID_LEN               10    

#define READER_PERSON_RTC_LEN               18  
#define READER_PERSON_MONEY_LEN             26 

#define READER_RFID_MOVE_TIME               12
//错误信息
#define READER_FRAME_FLAG_OK             0x00
#define READER_FRAME_FLAG_FAIL           0x01

#define READER_FRAME_INFO_OK             0x00
#define READER_FRAME_INFO_FAIL           0x04

#define READER_RSPFRAME_DAT_LEN         (2560)
#define READER_RSPFRAME_LEN             (READER_RSPFRAME_DAT_LEN + 32)


#define READER_RESFRAME_PERSON_FAIL		0x01
#define READER_RESFRAME_PERSON_TOTIME		0x02
#define READER_RESFRAME_PERSON_OK		0x04




#define READER_LED_RED_ALL_OFF           0x00000000
#define READER_LED_RED_ON                0x00000001
#define READER_LED_RED_OFF               0x00000002
#define READER_LED_GREEN_ON              0x00000004
#define READER_LED_GREEN_OFF             0x00000008
#define READER_LED_BLUE_ON               0x00000010
#define READER_LED_BLUE_OFF              0x00000020

#define READER_LEN_LIMIT_TIME           400


//
#define READER_LINK_TIME                25000//25000                   //125s
#define READER_OFF_LINE_TICK            2                      
#define READER_PERSON_BIND_FAIL         0x00
#define READER_PERSON_ALLERGY           0x01

#define NAME_NUM                        0x18
#define READER_ISO15693_BLOCK_BUF       2048
#define READER_UID_MAX_NUM              10

#define READER_OFF_LINE_DATA_NUM        50                                     //      暂定400条数据     
#define READER_DATA_UP_OK               0x01
#define READER_DATA_UP_FAIL             0x02


#define READER_RSP_NORMAL_DATA          0x01
#define READER_RSP_OFFLINE_DATA         0x02

#define REDAER_UART_TX_TICK              60
#define REDAER_UP_DATA_TICK              200
#define REDAER_UP_DATA_NUM               3

#define REDAER_UPMEALINFO_SUCC              0
#define REDAER_UPMEALINFO_FAIL              0x01
#define REDAER_UP_GPB_TICK              20
#define REDAER_UP_GPB_NUM               20


#define REDAER_LED_TIM                  0x0A

#define READER_MEAL_MASK_UID_TEST       0xFFFFFFFF
#define READER_MEAL_MASK_RTC_TEST       0x00000000




#define READER_LED_TIM                  50

#define READER_OFFLINE_DATA_BOUNDARY    2
typedef struct flashDevicePar{  
    u8 uiMode;                          //界面UI模式
    u8 rfCtrl;                           //射频默认状态
    u8 reWorkMode;                       //设备工作模式
    u8 rfWorkMode;                      //RFID工作模式
    u8 totalMode;                        //结算模式
    u8 ip[4];                            //IP地址
    u8 bud;                              //波特率
    u8 rfu1;
    u8 rfu2;
    u16 addr;                             //设备地址      
    u32 scoWt;                           //菜勺重量
    u32 crc;
}READER_DEVICE_PARAMETER;
extern READER_DEVICE_PARAMETER g_sDeviceParamenter;



typedef struct flashBootPar{  
    u16 appdata;
    u16 bud;
    u32 crc32;
}READER_BOOT_PARAMETER;



#define READER_OP_UID_MAX_NUM                   10

#define READER_IM_MODE_UID                  0x01    //读UID
#define READER_IM_MODE_RBLOCK               0x03    //读UID+BLOCK
#define READER_IM_BLOCK_OLD_NUM             4
#define READER_IM_BLOCK_NUM                 ISO15693_OPBLOCK_MAX_NUM
#define READER_IM_OP_OK                     0
#define READER_IM_OP_FAIL                   1

typedef struct readerImParams{
    u8 mode;
    u8 uidNum;
    u8 uid[ISO15693_SIZE_UID * READER_OP_UID_MAX_NUM];
    u8 blockNum;
    u8 blockAddr[READER_IM_BLOCK_NUM];
    u8 block[READER_OP_UID_MAX_NUM][READER_IM_BLOCK_NUM * ISO15693_SIZE_BLOCK];
    u8 result[READER_OP_UID_MAX_NUM];
    u32 opDelay;
}READER_IMPARAMS;
extern READER_IMPARAMS g_sReaderImParams;


typedef struct readerRspFrame{
    u16 len;
    u8 buffer[READER_RSPFRAME_LEN];
    u8 dtuFlag;
    u8 mode;
    u8 flag;
    u8 tag;
    u8 err;
    u8 state;
    u8 cmd;
    u8 repeat;
    u8 status;
    u8 succFlag;
    u16 destAddr;
    u32 rtcTime;
    u32 tick;
}READER_RSPFRAME;
extern READER_RSPFRAME g_sDeviceRspFrame;

extern READER_RSPFRAME g_sDeviceMealRspFrame;

typedef struct dishTagInfo{  
    char tagBuffer[LCM_TXT_TAG_LEN];
}DISH_TAG_INFO;


#define READER_DISH_INFO_NO_CHG     0x02

#define READER_DISH_INFO_CHG     0x01

typedef struct dishInfo{  
    u8 state;
    u8 dishName[NAME_NUM];      //菜名
    u8 tagNum;                  //标签数量
    u8 type;                    //种类
    u16 spec;                   // 规格
    u32 unitPrice;              //单价
    u32 peice;  
    DISH_TAG_INFO tagInfo[4];   //标签内容
}DISH_INFO;
extern DISH_INFO g_sDishTempInfo ;

typedef struct personInfo{  
    u8 tick;
    u8 state;
    u32 time;
    u32 money;
    u8 bind;
    u8 allergy;
    u32 crc32;
}PERSON_INFO;



typedef struct readerRfidInfo{  
    u8 blockNum;
    u8 blockAddr;
    u8 blockdata[READER_ISO15693_BLOCK_BUF];
    u8 uid[ISO15693_SIZE_UID];
    u8 tempState;               //判断出场进场交换
    u8 num;
    u8 state;
    u8 clash;
    u32 tick;                   
    u32 okTick;                 
    u32 errTick;
    u32 succesTick;
    u32 allTick;
}READER_RFID_INFO;

extern READER_RFID_INFO g_sReaderRfidTempInfo;

#define READER_RFID_ResetFrame(rcvFrame)               do{(rcvFrame).state = RFID_STAT_IDLE; rcvFrame.repat=0;}while(0)



#define READER_LINK_OK                  0x01
#define READER_LINK_FAIL                0x00
typedef struct readerInfo{  
    u32 wight;
    u32 total;
    u32 time;
    DISH_INFO dishInfo;
    PERSON_INFO personInfo;
    u32 crc32;	
}READER_INFO;

typedef struct readerLedInfo{
    u32 flag;
    u32 time;
    u32 state;
}READER_LED_INFO;

extern READER_LED_INFO g_sReaderLedInfo;

typedef struct readerOffLineInfo{  
    u16 num;
    u16 boundaryNum;
    u16 index;
    u8 readerOffLineData[READER_OFF_LINE_DATA_NUM];
    u32 crc;	
}READER_OFFLINE_INFO;




typedef struct readerDtuInfo{  
    u8 state;
    u8 flag;
    u8 txBuffrt[UART_BUFFER_MAX_LEN];
    u8 rxBuffrt[UART_BUFFER_MAX_LEN];
    u8 repat;
    u32 tick;
    u32 crc32;	
}READER_DTUINFO;


extern u32 g_nReaderState;
extern READER_DTUINFO g_sReaderDtuInfo;
extern READER_OFFLINE_INFO g_sReaderOffLineInfo;
extern u32 g_nRtcTime;
extern u32  g_nTickLink ;
extern u8 g_nTempLink;
extern BOOL g_nReaderLink;
extern READER_INFO g_sRaderInfo;
extern char g_nBufTxt[LCM_TXT_LEN_MAX];
extern char g_nBufTxt1[LCM_TXT_LEN_MAX];



extern BOOL g_nReaderGpbState;
extern u8 g_aReaderISO15693Uid[READER_OP_UID_MAX_NUM * ISO15693_SIZE_UID]; 
void Reader_Delayms(u32 n);
void Reader_Init();
void Reader_DisplayIp(READER_DEVICE_PARAMETER *pBuffer);



void Reader_ChgKey(u8 stat);
void Reader_DisplayLable(DISH_INFO *pBuffer);


void Reader_ChkLink(BOOL link);
void Reader_DisplayDish(DISH_INFO *pDishInfo, GPB_INFO *gpbInfo, LCM_INFO *pLcmInfo);
void Reader_ChgPage(LCM_INFO *pLcmInfo);
void Reader_ChkMoney(u32 value, char *buf);
void Reader_Font_ChgCorol(u8 background);
void Reader_InVoTags();
void Reader_OffLineClear();
void Fram_Demo();
void Reader_ChgStat(u8 lineState);                                                                                                  
void Reader_Chk_KeyValue(u8 value);
void Reader_Normal_Mode();

BOOL Reader_ReadOffLineDataNum(void) ;
BOOL Reader_ReadOffLineDatas(u16 addr, u16 size, u8 *pBuffer)   ;
BOOL Reader_WriteOffLineDatas(u16 addr, u16 size, u8 *pBuffer)   ;
BOOL Reader_WriteOffLineDataNum(void);
BOOL Reader_Rfid_Init();
BOOL Reader_WriteDeviceParamenter(void);
BOOL Reader_ChkReUid(u8 *pRUid, u8 *pTUid);
BOOL Reader_ReadDeviceParamenter(void);

BOOL Reader_DtuUartFrame(u8 *pFrame, READER_DTUINFO *pDtuInfo, u16 len);


//u8 Reader_RfidGetValue(u8 mode, READER_RFID_INFO *pRfidInfo);
u8 Reader_RfidTask(u8 mode, PERSON_INFO *personInfo, READER_RFID_INFO *rfidInfo);
u8 Reader_RfidGetValue(u8 *pBuffer, READER_RFID_INFO *pRfidInfo);
u8 Device_ResponseFrame(u8 *pParam, u8 len, READER_RSPFRAME *pOpResult);
u8 Reader_DisplayTest(DISH_INFO *pDishInfo, WIGHT_INFO *witghInfo)  ;
u8 Reader_Format_Meal(u8 type,u8 *puid, READER_RSPFRAME *pOpResult, u32 rtc, u32 witgh);
u8 Reader_Format_Heart( READER_RSPFRAME *pOpResult, u32 rtc,u32 witgh);
u8 Reader_Format_Cfg(u8 mode, READER_RSPFRAME *pOpResult, READER_DEVICE_PARAMETER *pParameter);
u8 Reader_Lable(u8 *pBuffer, DISH_INFO *pDishInfo);
u8 Reader_Format_GetPerson(u8 *puid,  READER_RSPFRAME *pOpResult, u32 rtc);
u16 Reader_ProcessUartFrame(u8 *pFrame, u16 len);
u16 Reader_Seek_Id(u8 *pBuffer);
u16 Reader_Seek_Num(u8 *pBuffer);
u16 Reader_Seek_Inq(u8 *pBuffer) ;
u32 Reader_GetUartBaudrate();
u32 Reader_GetWight(GPB_INFO *gpbInfo);

#endif
