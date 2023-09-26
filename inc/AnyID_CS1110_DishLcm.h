#ifndef _ANYID_CS1110_DISHLCM_H
#define _ANYID_CS1110_DISHLCM_H

#include "AnyID_CS1110_DishLcm_HL.h"

//  x=txt
#define LCM_TXT_DEVICE_OFF_LINK    	"断开连接"
#define LCM_TXT_NO_BIND_TAG			"未绑定托盘"
#define LCM_TXT_PUT_TAG				"请放入托盘"
#define LCM_TXT_ALLERY				"您对该菜品过敏"
#define LCM_TXT_DEVICE_ERR    		"当前设备不可用"
















//
#define LCM_STAT_IDLE 	        0x00
#define LCM_STAT_RCV 			0x01
#define LCM_STAT_OVR 			0x04
#define LCM_STAT_TX 			0x08
#define LCM_STAT_TX_IDLE        0x10
#define LCM_STAT_WAIT           0x20

#define LCM_DISH_FRAME_POS_LEN              2
#define LCM_DISH_FRAME_DATA_MIN_LEN         3
#define LCM_DISH_FRAME_MIN_LEN              6
#define LCM_DISH_BUFFER_MAX_LEN             (64 + LCM_DISH_FRAME_MIN_LEN)
#define LCM_DISH_FRAME_FLAG_HEAD1           0x5A
#define LCM_DISH_FRAME_FLAG_HEAD2           0xA5
                                                
                                              
#define LCM_DISH_FRAME_CMD_WREG              0x80
#define LCM_DISH_FRAME_CMD_RREG              0x81
#define LCM_DISH_FRAME_CMD_WVAR              0x82
#define LCM_DISH_FRAME_CMD_RVAR              0x83
                                                
#define LCM_DISH_REG_ADDR_PICID              0x03        //显示页面ID寄存器
                                                
#define LCM_DISH_TX_MODE_MID                 0
#define LCM_DISH_TX_MODE_LEFT                1
#define LCM_DISH_TX_MODE_RIGHT               2
                 
#define LCM_DISH_TXBUF_MAX_LEN               256

//颜色是反过来的----------注意
#define LCM_DISH_SELECTED_COLOR             0xE0FF
#define LCM_DISH_UNSELECTED_COLOR           0x8E73


#define LCM_FONT_COROL_WHITE                0xFFFF       
#define LCM_FONT_COROL_BLACK                0x0000
#define LCM_FONT_COROL_GREEN                0xE007
#define LCM_FONT_COROL_RED                  0x00F8
#define LCM_DISH_FUN_TXT_SIZE               14


#define LCM_DISH_TXT_LEN_MAX                32

//显示页面
#define LCM_PAGE_MAIN_WHITE		0
#define LCM_PAGE_INFO_WHITE		1
#define LCM_PAGE_LINK_WHITE		2
#define LCM_PAGE_NULL_WHITE		3

#define LCM_PAGE_MAIN_BLACK		10
#define LCM_PAGE_INFO_BLACK		11
#define LCM_PAGE_LINK_BLACK		12
#define LCM_PAGE_NULL_BLACK		13

#define LCM_PAGE_TEST   		20
//页面切换
#define LCM_CHG_PAGE_ADDR               0x0084    //系统变量接口地址，固定页面切换
#define LCM_CHG_PAGE_FIX_FRAME          0x5A01

//图标控制
  
#define LCM_CTR_ICO_DISPLAY_L           0x0000
#define LCM_CTR_ICO_DISPLAY_H           0x0001
#define LCM_CTR_ICO_DISPLAY_N           0x0002

#define LCM_ICO_LINK_ADDR               0X3000 
#define LCM_ICO_LOGO_ADDR               0X3010                     

//显示文字
#define LCM_INFO_TXT_SIZE                14 
#define LCM_TIP_TXT_SIZE                 18 
#define LCM_TXT_ADDR_LINK                0x1010
#define LCM_TXT_ADDR_IP                  0x1030


#define LCM_TXT_ADDR_NAME                0x1040
#define LCM_TXT_ADDR_INFO                0x1050
#define LCM_TXT_ADDR_TAG_1               0x1060
#define LCM_TXT_ADDR_TAG_2               0x1070
#define LCM_TXT_ADDR_TAG_3               0x1080
#define LCM_TXT_ADDR_TAG_4               0x1090
#define LCM_TXT_ADDR_PEICE               0x1100
#define LCM_TXT_ADDR_SPEC                0x1110
#define LCM_TXT_ADDR_WIGHT               0x1120
#define LCM_TXT_ADDR_MONEY               0x1130
#define LCM_TXT_ADDR_TOTAL               0x1140


#define LCM_TXT_ADDR_TEST_1              0x1210
#define LCM_TXT_ADDR_TEST_2              0x1220
#define LCM_TXT_ADDR_TEST_3              0x1230


#define LCM_TXT_ADDR_TEST_BUTTON_1       0x1190
#define LCM_TXT_ADDR_TEST_BUTTON_2       0x1250
#define LCM_TXT_ADDR_TEST_BUTTON_3       0x1240

#define LCM_TXT_ADDR_TEST_TAG_STAT       0x1260

#define LCM_TXT_ADDR_LINK2		 0x1270

#define LCM_TXT_ADDR_LINK_STATE          0x1320
#define LCM_TXT_ADDR_SCOOP_WTGHT         0x1340
#define LCM_TXT_ADDR_SCOOP_TXT           0x1330

#define LCM_TXT_ADDR_SCOOP_MONEY         0x1360
#define LCM_TXT_ADDR_ARRERGY             0x1450

#define LCM_TXT_ADDR_NAME_2              0x1640
#define LCM_TXT_ADDR_YUAN0               0x1380
#define LCM_TXT_ADDR_YUAN1               0x1580
#define LCM_TXT_ADDR_KG                  0x1590

#define LCM_TXT_ADDR_SOFT_VERSION        0x1850
#define LCM_TXT_ADDR_RFID_SOFT_VERSION   0x1860
//文字颜色


#define LCM_TXT_ADDR_LINK_COLOR          0x2010
#define LCM_TXT_ADDR_LINK_TXT_COLOR      0x2020
#define LCM_TXT_ADDR_MAIN_COLOR          0x2030

#define LCM_TXT_ADDR_NAME_COLOR          0x2040
#define LCM_TXT_ADDR_INFO_COLOR          0x2050
#define LCM_TXT_ADDR_TAG_1_COLOR         0x2060
#define LCM_TXT_ADDR_TAG_2_COLOR         0x2070
#define LCM_TXT_ADDR_TAG_3_COLOR         0x2080
#define LCM_TXT_ADDR_TAG_4_COLOR         0x2090
#define LCM_TXT_ADDR_PEICE_COLOR         0x2100
#define LCM_TXT_ADDR_SPEC_COLOR          0x2110
#define LCM_TXT_ADDR_WIGHT_COLOR         0x2120
#define LCM_TXT_ADDR_MONEY_COLOR         0x2130
#define LCM_TXT_ADDR_TOTAL_COLOR         0x2140
#define LCM_TXT_ADDR_SCOOP_COLOR         0x2150

#define LCM_TXT_ADDR_NAME_2_COLOR        0x2640
#define LCM_TXT_ADDR_NO_MEAL_COLOR       0x2270

#define LCM_TXT_ADDR_SCOOP_WITGH_COLOR   0x2330
#define LCM_TXT_ADDR_SCOOP_MONEY_COLOR   0x2350

#define LCM_TXT_ADDR_LINK_INFO_COLOR     0x2320



#define LCE_TXT_ADDR_KEY_UP             0x2610
#define LCE_TXT_ADDR_KEY_MIDDLE         0x2620
#define LCE_TXT_ADDR_KEY_DOWN           0x2630

#define LCE_TXT_ADDR_LINE_STAT          0x2260


#define LCM_TXT_TEST_LEN                 8
#define LCM_TXT_TEST_BUTTON_LEN          3
#define LCM_TXT_OFFSET_NULL              0

#define LCM_TXT_SUCCES_FULL              10000
//图片变量显示
#define LCM_ICO_ADDR_LOGO                0x3010
#define LCM_ICO_ADDR_GROUND              0x3020
#define LCM_ICO_ADDR_PALLET              0x3030

#define LCM_ICO_ADDR_TAG_1               0x3040
#define LCM_ICO_ADDR_TAG_2               0x3050
#define LCM_ICO_ADDR_TAG_3               0x3060
#define LCM_ICO_ADDR_TAG_4               0x3070

#define LCM_ICO_ADDR_WARN                0x3080
#define LCM_ICO_ADDR_FAIL_LINK           0x3090
#define LCM_ICO_ADDR_ALLERGY             0x5150
#define LCM_ICO_ADDR_ALLERGY_BACK        0x5140
#define LCM_ICO_ADDR_OFFLINE_BACK        0x5060

//背景亮度调节
#define LCM_BACKGROUND_ADDR              0x5432

#define LCM_BACKGROUND_ADDR_WITGH        0x5000
#define LCM_BACKGROUND_ADDR_MONEY        0x5010
#define LCM_BACKGROUND_ADDR_TOTAL        0x5020
#define LCM_BACKGROUND_ADDR_TIP          0x5030

#define LCM_BACKGROUND_ADDR_INFO         0x5040
#define LCM_BACKGROUND_ADDR_INFO         0x5040
#define LCM_BACKGROUND_ADDR_ALLERGY      0x5050


#define LCM_BACKGROUND_STR_NORMAL       0xFF
#define LCM_BACKGROUND_STR_LDRAK        0xA2
#define LCM_BACKGROUND_STR_HDRAK        0x22 
//屏幕软硬版本
#define LCM_CMD_GET_INFO_ADDR           0x000F
//
#define LCM_TXT_LEN_MAX                 32
#define LCM_TXT_TAG_LEN                 8


#define LCM_FLAG_PAGE_NO_CHG            0xFF

#define LCM_FLAG_PAGE_MAIN_CHG          0x00
#define LCM_FLAG_PAGE_INFO_CHG          0x01
#define LCM_FLAG_PAGE_LINK_CHG          0x02
#define LCM_FLAG_PAGE_NULL_CHG          0x03
#define LCM_FLAG_PAGE_SET_CHG           0x04   

#define LCM_FLAG_PAGE_TEST            21  //20

#define LCM_UP_TICK                     10
#define LCM_UP_NUM                      3
#define Lcm_IsRcvFrame(rcvFrame)           ((rcvFrame).state == LCM_STAT_OVR)
#define Lcm_ResetFrame(rcvFrame)           do{(rcvFrame)->state = LCM_STAT_IDLE; (rcvFrame)->length = 0; }while(0)
#define Lcm_DishGetFrameLength(p)           (((p)[LCM_DISH_FRAME_POS_LEN]) + 3)
                                                
#define Lcm_DishIncIdleTime(t, rcvFrame)    do{\
                                                if((rcvFrame).state & LCM_DISH_FLAG_RCV)\
                                                {\
                                                    (rcvFrame).idleTime += (t);\
                                                    if((rcvFrame).idleTime >= (2 * t))\
                                                    {\
                                                        (rcvFrame).state = LCM_DISH_STAT_TO;\
                                                    }\
                                                }\
                                            }while(0)
                                                
#define Lcm_DishIsRcvFrame(rcvFrame)        ((rcvFrame).state == LCM_DISH_STAT_TO || (rcvFrame).state == LCM_DISH_STAT_END)
#define Lcm_DishResetFrame(rcvFrame)        (rcvFrame).state = LCM_DISH_STAT_IDLE

 

#define Lcm_ChkChg(r)                        (r->flag != r->page)
#define Lcm_ChkPage(r)                       (g_sLcmInfo.page == r)
#define Lcm_ChgPage(r)                       (r->page = r->flag)
#define Lcm_SetPage(f)                       do{g_sLcmInfo.flag = f;}while(0)                                             
#define Lcm_ChkLink(r)                       do{\
                                                    if(r.flag == LCM_FLAG_PAGE_LINK_CHG)\
                                                    {\
                                                        r.flag = LCM_FLAG_PAGE_NULL_CHG ;\
                                                    }\
                                               }while(0)


#define Lcm_DishTableWarning(pTxt) Lcm_DishWriteTxt(LCM_DISH_VAR_ADDR_TBLWRN, LCM_DISH_VAR_LEN_TBLWRN, (pTxt), LCM_DISH_TX_MODE_MID, 0);                                                

    

                                              
//******************************************************************************
//变量声明
//******************************************************************************
typedef struct lcmDishRcvFrame{
    u8 state;
    u16 length;
    u32 idleTime;
    u8 buffer[LCM_DISH_BUFFER_MAX_LEN];
}LCM_DISH_RCVFRAME;
extern LCM_DISH_RCVFRAME g_sLcmDishRcvFrame;
extern LCM_DISH_RCVFRAME g_sLcmDishRcvTempFrame;


typedef struct lcmInfo{
    u8 page;
    u8 flag;
}LCM_INFO;


                                             

typedef struct lcmDishTxFrame{
    u8 repat;
    u8 frame[LCM_DISH_TXBUF_MAX_LEN];
    u8 len;
    u32 tick;
}LCM_DISH_TXFRAME;

extern LCM_INFO g_sLcmInfo;
extern u8 g_nTempPage;
extern LCM_DISH_TXFRAME g_sLcmDishTxFrame;            
  

void Lcm_Init();
void Lcm_DishTableBusy(void);
void Lcm_SelectPage(u16 page);
void Lcm_DishTableClear(void);
void Lcm_DishSelectPage(u16 page);
void Lcm_DishInitCfgAddr(u8 addr);
void Lcm_DishInitCfgGroup(u8 group);
void Lcm_SelectIco(u16 ico,u16 addr);
void Lcm_DishClearPage0(char *pTitle);
void Lcm_BackgroundCtr(u16 data,u16 addr);
void Lcm_DishWriteColor(u16 addr, u16 color);
void Lcm_DishTableIdle(u8 rssi, u16 tblId, char *pTbleName);
void Lcm_DishReceiveFrame(u8 byte, LCM_DISH_RCVFRAME *pRcvFrame);
void Lcm_DishWriteTxt(u16 addr, u8 bufLen, char *pTxt, u8 mode, u8 offset);
void Lcm_DishWriteUniCodeTxt(u16 addr, u8 bufLen, char *pTxt, u8 mode, u8 offset);

u8 Lcm_ChgPageFormatFrame(u16 page,u8 *pFrame);
u8 Lcm_FormatFrame(u16 data,u16 addr,u8 *pFrame);
u8 Lcm_DishWriteRegFrame(u8 addr, u8 *pReg, u8 num, u8 *pFrame);
u8 Lcm_DishWriteVarFrame(u16 addr, u8 *pVar, u16 varLen, u8 *pFrame);

u8 Lcm_DishWriteVarUniCodeFrame(u16 addr, u8 *pVar, u16 varLen, u8 *pFrame);

#endif
