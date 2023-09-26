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
#define READER_STAT_CHK_PERSON_INFO        0x00020000



#define DEVICE_OFFDATA_LIMIT_TIME		  720000/3   // 12 hours
#define RFID_ANT_OPEN                     0x00
#define RFID_ANT_CLOSE                    0x01

#define READER_TOTAL_MODE_AIR             0x00
#define READER_TOTAL_MODE_AFTER           0x01


#define READER_UI_MODE_WHITE             0x00
#define READER_UI_MODE_BLACK             0x0A


#define READER_WITHT_LIMIT_VALUE			5

#define READER_RSPFRAME_FLAG_OK          0x00
#define READER_RSPFRAME_FLAG_FAIL        0x01
#define READER_OPTAG_RESPONSE_NOERR      0x00
#define READER_OPTAG_RESPONSE_TAGERR     0x01
#define READER_OPTAG_RESPONSE_CRCERR     0x02
#define READER_OPTAG_RESPONSE_NORSP      0x03
#define READER_OPTAG_RESPONSE_PARERR     0x04



#define READER_OVER_WIGHT_VALUE			1
#define READER_OVER_WIGHT_TICK			10

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
#define READER_CMD_CHG_IP                   0x55
#define READER_CMD_DRU_CMD                  0xFE

#define READER_CMD_SET_CFG                  0x20
#define READER_CMD_GET_CFG                  0x21
#define READER_CMD_GET_PERSON_INFO          0x22
#define READER_CMD_GET_MEAL_INFO            0x23
#define READER_CMD_GET_HEART                0x25




#define READER_CMD_DRU_RFID                 1
#define READER_CMD_DRU_WIGHT                2
#define READER_CMD_DRU_LCM                  3
#define READER_CMD_DRU_RESET                10
#define READER_CMD_DRU_LCD                  0x33
#define READER_CMD_DRU_GET_KEY              0x34


#define READER_SFG_IP_LEN                       0x20
#define READER_DTU_FLAG_VOICD                   4
#define READER_DTU_FLAG_LCM                     3
#define READER_DTU_FLAG_GPB                     2
#define READER_DTU_FLAG_READER					1

#define READER_DTU_FLAG_NULL					0

#define DEVICE_ADDR_NULL						0x00
#define DEVICE_ADDR_READER						0x01
#define DEVICE_ADDR_WIGHT						0x02
#define DEVICE_ADDR_LCM							0x04

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

#define READER_RFID_MOVE_TIME               5
//错误信息
#define READER_FRAME_FLAG_OK             0x00
#define READER_FRAME_FLAG_FAIL           0x01

#define READER_FRAME_INFO_OK             0x00
#define READER_FRAME_INFO_FAIL           0x04

#define READER_RSPFRAME_DAT_LEN         (2560)
#define READER_RSPFRAME_LEN             (READER_RSPFRAME_DAT_LEN + 32)


#define READER_RESFRAME_PERSON_FAIL		0x01
#define READER_RESFRAME_PERSON_TOTIME	0x02
#define READER_RESFRAME_PERSON_OK		0x04




#define READER_LED_RED_ALL_OFF           0x00000000
#define READER_LED_RED_ON                0x00000001
#define READER_LED_RED_OFF               0x00000002
#define READER_LED_GREEN_ON              0x00000004
#define READER_LED_GREEN_OFF             0x00000008
#define READER_LED_BLUE_ON               0x00000010
#define READER_LED_BLUE_OFF              0x00000020

#define READER_LEN_LIMIT_TIME           400



//参数

#define READER_DEFAULT_PARAMETER_SOWT	20		
#define READER_DEFAULT_PARAMETER_IP		0xFF		

//
#define READER_LINK_TIME                24000		//30s		//25000//25000                   //125s
#define READER_OFF_LINE_TICK            1                      
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

#define REDAER_UART_TX_TICK              100
#define REDAER_UP_DATA_TICK              60
#define REDAER_UP_DATA_NUM               3

#define READER_WIGHT_MAX_VALUE			0xFFFFFFFF
#define REDAER_UPMEALINFO_SUCC			0
#define REDAER_UPMEALINFO_FAIL			0x01
#define REDAER_UP_GPB_TICK              20
#define REDAER_UP_GPB_NUM               20

#define READER_SOFT_VERSON_LEN			15

#define REDAER_LED_TIM                  0x0A

#define READER_MEAL_MASK_UID_TEST       0xFFFFFFFF
#define READER_MEAL_MASK_RTC_TEST       0x00000000


#define READER_LIMIT_RE_PERSONINFO		40

#define READER_LED_TIM                  300

#define READER_OFFLINE_DATA_BOUNDARY    9
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
	u8 deviceFlag;
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

#define READER_PERSON_BIND_MASK_FAIL		0
#define READER_PERSON_BIND_MASK_OK			1
#define READER_PERSON_BIND_MASK_OP_TIME     2

#define READER_PERSON_INFO_RSP_FAIL             0x00
#define READER_PERSON_INFO_RSP_OK               0x01
#define READER_PERSON_INFO_RSP_OP               0x02
#define READER_PERSON_INFO_RSP_LOADING          0x04


typedef struct personInfo{
	u8 flag;
    u8 successFlag;
    u8 tick;
    u8 state;
    u8 bind;
    u8 allergy;
    u32 time;
    u32 money;
	u32 limitTick;
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
	u32 clashTicks;
	u32 clashTick;
    u32 okTick;                 
    u32 errTick;
    u32 succesTick;
    u32 allTick;
}READER_RFID_INFO;

extern READER_RFID_INFO g_sReaderRfidTempInfo;
#define READER_LINK_OK                  0x01
#define READER_LINK_FAIL                0x02
#define READER_RFID_ResetFrame(rcvFrame)               do{(rcvFrame).state = RFID_STAT_IDLE; rcvFrame.repat=0;}while(0)


#define Reader_ChkValue(u, v)     					 ({\
														int sampleValue = 0;\
														if(u & GPB_WITGH_MASK_VALUE)\
														{\
															sampleValue = -(u & GPB_WITGH_MASK_VALUE_ABS);\
														}\
														else \
														{\
															sampleValue = (u & GPB_WITGH_MASK_VALUE_ABS);\
														}\
														(v - sampleValue);\
     					 							})\
														
#define Device_VoiceApoFrame(cmd,opt,repat,mode,vn)     do{\
															if(g_sSoundInfo.txBuf.num <  SOUND_OP_NUM)\
															{\
																g_sSoundInfo.txBuf.cd[g_sSoundInfo.txBuf.num] = cmd;\
																g_sSoundInfo.txBuf.to[g_sSoundInfo.txBuf.num] = opt;\
																g_sSoundInfo.txBuf.id[g_sSoundInfo.txBuf.num] = vn;\
																g_sSoundInfo.txBuf.repeat[g_sSoundInfo.txBuf.num] = repat;\
																g_sSoundInfo.txBuf.op[g_sSoundInfo.txBuf.num++] = mode;\
																if(g_sSoundInfo.state == SOUND_STAT_IDLE || g_sSoundInfo.txBuf.op[g_sSoundInfo.txBuf.index] != mode)\
																{\
																	g_sSoundInfo.txBuf.index = g_sSoundInfo.txBuf.num - 1;\
																	g_sSoundInfo.state = SOUND_STAT_TX;\
																}\
															}\
														}while(0)											 

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
	u16 id;
    u16 num;
    u16 boundaryNum;
    u16 index;
    u8 readerOffLineData[READER_OFF_LINE_DATA_NUM];
    u32 crc;	
}READER_OFFLINE_INFO;



//队列数据

#define READER_CMD_FRAME_LEN_RSP_OK            0xFF
#define READER_CMD_FRAME_LEN_SET_CFG           7
#define READER_CMD_FRAME_LEN_MEAL              12
#define READER_CMD_FRAME_LEN_PERSON            21
#define READER_WITHT_SCOTW_MIN                 5

#define READER_MEAL_NAME_LEN_MIN                0
#define READER_MEAL_NAME_LEN_MAX                64
#define READER_MEAL_TAG_NAME_LEN_MIN            36

#define READER_DATA_NUM                         32
#define READER_DATA_SUB_LEN                     64


#define READER_DEVICE_STAT_IDLE                 0
#define READER_DEVICE_STAT_TX                   1
#define READER_DEVICE_STAT_WAIT                 2
#define READER_DEVICE_STAT_DELAY                3
#define READER_DEVICE_STAT_RX                   4
#define READER_DEVICE_STAT_NEXT_STEP            5

#define READER_DEVICE_DATA_REPAT_TICK           2
#define READER_DEVICE_DELAY_TIME                20
#define READER_DEVICE_OP_TIME                   60

#define Reader_UartDelay(t)                      {g_sReaderData.state = READER_DEVICE_STAT_DELAY; g_sReaderData.tick = t;}
#define Reader_UartAddList(pBuffer,lenth)          do{\
                                                      	if(g_sReaderData.num <= READER_DATA_NUM)\
                                                      	{\
															memcpy(g_sReaderData.data[g_sReaderData.num].buffer, pBuffer, lenth);\
															g_sReaderData.data[g_sReaderData.num].rtcTick = g_nRtcTime;\
															g_sReaderData.data[g_sReaderData.num].len = lenth;\
															g_sReaderData.num++;\
															if(g_sReaderData.state == READER_DEVICE_STAT_IDLE)\
															{\
															   g_sReaderData.state = READER_DEVICE_STAT_TX;\
															}\
                                                      	}\
                                                    }while(0)
#define READER_DEVICE_DATA_TX_RELUST_OK          0x01
#define READER_DEVICE_DATA_TX_RELUST_FAIL           0

                                                      
#define READER_DATA_PRIORITY_HIGH              0
#define READER_DATA_PRIORITY_LOW               1
                                                      
#define READER_DATA_SIGN_IDLE                  0
#define READER_DATA_SIGN_BUSY                  1                                                   
#define READER_DATA_SIGN_DATA_ADD              2
#define READER_DATA_SIGN_UP_OK                 3 
                                                      
typedef struct readerDataSub{
  u8 len;
  u8 buffer[READER_DATA_SUB_LEN];
  u32 rtcTick;
}READER_DATA_SUB;

typedef struct readerData{
  u8 sign;
  u8 flag;
  u8 relust;
  u8 state;
  u8 index;
  u8 num;
  u8 repat;
  u8 priority;
  u32 tick;
  READER_DATA_SUB data[READER_DATA_NUM];
}READER_DATA;

extern READER_DATA g_sReaderData;
  
//



extern BOOL g_nReaderLink;
extern BOOL g_nReaderGpbState;
extern u8 g_nTempLink;
extern u16 g_nTempId;
extern u32 g_nReaderState;
extern u32 g_nRtcTime;
extern u32  g_nTickLink ;
extern int g_nReaderWightValue;
extern u32 g_nOffLineLimitTime;
extern READER_OFFLINE_INFO g_sReaderOffLineInfo;
extern READER_INFO g_sRaderInfo;
extern char g_nBufTxt[LCM_TXT_LEN_MAX];
extern char g_nBufTxt1[LCM_TXT_LEN_MAX];
extern u8 g_aReaderISO15693Uid[READER_OP_UID_MAX_NUM * ISO15693_SIZE_UID]; 


void Reader_Delayms(u32 n);
void Reader_Init();
void Reader_DisplayIp(READER_DEVICE_PARAMETER *pBuffer);
void Reader_DisplayLable(DISH_INFO *pBuffer);
void Reader_DisplayDish(DISH_INFO *pDishInfo, GPB_INFO *gpbInfo, LCM_INFO *pLcmInfo);
void Reader_ChgPage(LCM_INFO *pLcmInfo);
void Reader_ChkMoney(u32 value, char *buf);
void Reader_FontChgCorol(u8 background);
void Fram_Demo();
void Reader_ChgStat(u8 lineState);                                                                                                  
void Reader_ChkKeyValue(u8 value);
void Reader_NormalMode();
void Reader_ResolveWitgh();
void Reader_OLInfoHanld();

BOOL Reader_ReadOffLineDataNum(void) ;
BOOL Reader_ReadOffLineDatas(u16 addr, u16 size, u8 *pBuffer)   ;
BOOL Reader_WriteOffLineDatas(u16 addr, u16 size, u8 *pBuffer)   ;
BOOL Reader_WriteOffLineDataNum(void);
BOOL Reader_RfidInit();
BOOL Reader_WriteDeviceParamenter(void);
BOOL Reader_ChkReUid(u8 *pRUid, u8 *pTUid);
BOOL Reader_ReadDeviceParamenter(void);

//u8 Reader_RfidGetValue(u8 mode, READER_RFID_INFO *pRfidInfo);
u8 Reader_RfidTask(u8 mode, PERSON_INFO *personInfo, READER_RFID_INFO *rfidInfo);
u8 Reader_RfidGetValue(u8 *pBuffer, READER_RFID_INFO *pRfidInfo);
u8 Device_ResponseFrame(u8 *pParam, u8 len, READER_RSPFRAME *pOpResult);
u8 Reader_DisplayTest(DISH_INFO *pDishInfo, WIGHT_INFO *witghInfo)  ;
u8 Reader_FormatMeal(u8 type,u8 *puid, READER_RSPFRAME *pOpResult, u32 rtc, u32 witgh);
u8 Reader_FormatHeart( READER_RSPFRAME *pOpResult, u32 rtc,u32 witgh);
u8 Reader_FormatCfg(u8 mode, READER_RSPFRAME *pOpResult, READER_DEVICE_PARAMETER *pParameter);
u8 Reader_Lable(u8 *pBuffer, DISH_INFO *pDishInfo);
u8 Reader_FormatGetPerson(u8 *puid,  READER_RSPFRAME *pOpResult, u32 rtc);
u8 Reader_SearchCode(u8 lenth, u8 code, char *pBuf);
u16 Reader_ProcessUartFrames(u8 *pFrame, u16 len);
u16 Reader_SeekId(u8 *pBuffer);
u16 Reader_SeekNum(u8 *pBuffer);
u16 Reader_SeekInq(u8 *pBuffer) ;
u32 Reader_GetUartBaudrate();
u32 Reader_GetWight(GPB_INFO *gpbInfo);


#endif
