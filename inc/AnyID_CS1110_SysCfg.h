#ifndef _ANYID_CS1110_SYSCFG_H
#define _ANYID_CS1110_SYSCFG_H

#include "AnyID_CS1110_Config.h"


#include "AnyID_CS1110_Reader.h"



extern u32 g_nSysState;
#define SYS_STAT_IDLE                   0x00000001	        //
#define SYS_STAT_RUNLED                 0x00000002
#define SYS_STAT_PUT_TAG                0x00000004
#define SYS_STAT_LINE_OFF               0x00000008
#define SYS_STAT_CLASH_TAG              0x00000010  
#define SYS_STAT_UARTTX                 0x00000020
#define SYS_STAT_BIND_TAG               0x00000040
#define SYS_STAT_UART_WAIT              0x00000080
#define SYS_STAT_CHK_UPDATA             0x00000100
#define SYS_STAT_UARTRX                 0x00000200
#define SYS_STAT_OVER_OFFLINE           0x00000400
#define SYS_STAT_RFID_KEEP              0x00000800
#define SYS_STAT_MODE_NORMAL            0x00001000
#define SYS_STAT_MODE_TEST              0x00020000

#define SYS_STAT_G_LED_DELAY            0x10000000
#define SYS_STAT_B_LED_DELAY            0x20000000
#define SYS_STAT_R_LED_DELAY            0x40000000




#define SYS_STAT_KEY                    0x00400000
//#define SYS_STAT_GET_PERSONINFO_FAIL    0x00000800
#define SYS_STAT_LCM_FRESH              0x80000000
#define SYS_STAT_SOUND_FAIL             0x00002000
#define SYS_STAT_GPB_FAIL               0x00004000
#define SYS_STAT_LCM_FAIL		0x00008000
#define SYS_STAT_AUTO_RFID              0x00800000
#define SYS_STAT_TEST                   0x00200000
#define SYS_STAT_CHK_LINK               0x02000000

#define SYS_STAT_OFF_LINE               0x04000000
#define SYS_STAT_GPB_SAMPLE             0x08000000
#define SYS_STAT_LCM_TX                 0x00040000
#define SYS_STAT_UID_GET                0x00100000
#define SYS_STAT_UID_OUT                0x00010000
#define SYS_STAT_TEST_LCM_TAG           0x01000000





#define SYS_LED_DELAY                   20
extern const PORT_INF SYS_CFG_CHK;      
#define Sys_CheckCfgMode()              ((SYS_CFG_CHK.Port->IDR & SYS_CFG_CHK.Pin) == 0x0000)

extern const PORT_INF SYS_LED_RED;
#define Sys_LedRedOff()                  SYS_LED_RED.Port->BRR = SYS_LED_RED.Pin
#define Sys_LedRedOn()                  SYS_LED_RED.Port->BSRR = SYS_LED_RED.Pin

extern const PORT_INF SYS_LED_GREEN;
#define Sys_LedGreenOff()                  SYS_LED_GREEN.Port->BRR = SYS_LED_GREEN.Pin
#define Sys_LedGreenOn()                 SYS_LED_GREEN.Port->BSRR = SYS_LED_GREEN.Pin

extern const PORT_INF SYS_LED_BLUE;
#define Sys_LedBlueOff()                  SYS_LED_BLUE.Port->BRR = SYS_LED_BLUE.Pin
#define Sys_LedBlueOn()                 SYS_LED_BLUE.Port->BSRR = SYS_LED_BLUE.Pin

#define SYS_CHECK_CFGMODE_TIM           200
void Sys_Delayms(u32 n);
void Sys_CfgClock(void);
void Sys_CfgPeriphClk(FunctionalState state);
void Sys_CfgNVIC(void);

void Sys_CtrlIOInit(void);
u8 Sys_GetDevAddr(void);

void Sys_Init(void);
void Sys_KeyTask();
void Sys_SoundTask();
void Sys_GPBTask();
void Sys_LedTask(void);
void Sys_RfidTask();
void Sys_UartTask(void);
void Sys_LcmTask();
void Sys_TestTask();
void Sys_ReaderTask();
#endif
