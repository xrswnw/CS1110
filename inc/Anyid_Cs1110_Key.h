#ifndef _ANYID_CS1110_KEY_H
#define _ANYID_CS1110_KEY_H

#include "AnyID_CS1110_Key_HL.h"
#include "AnyID_CS1110_Reader.h"
#include "AnyID_CS1110_Sound.h"
#include "AnyID_CS1110_SysCfg.h"

#define KEY_STAT_IDLE       	0x00
#define KEY_STAT_FILEER     	0x01
#define KEY_STAT_OK         	0x04
#define KEY_STAT_DLY            0x08
#define KEY_STAT_SAMPLE     	0x10
#define KEY_STAT_ERROR          0x20

#define KEY_MDOE_MAIN       	0x00
#define KEY_MDOE_TEST     	0x01

#define KEY_VALUE_LONG_RIGHT    0x24
#define KEY_VALUE_LONG_MID      0x12
#define KEY_VALUE_LONG_LEFT     0x09
#define KEY_VALUE_LEFT          0x04
#define KEY_VALUE_MID           0x02
#define KEY_VALUE_RIGHT         0x01
#define KEY_LONG_SAMPLE_TIME    40
#define KEY_LONG_PRESS_TIME     200
#define KEY_LONG_ERR_TIME       2000





typedef struct keyValue{
    u8 state;
    u8 mode;
    u8 value;
    u32 tick;
}KEY_VALUE;
extern KEY_VALUE g_sKeyValue;

void Key_Init();
void Key_Control(KEY_VALUE *pBuffer);
void Key_Value_Main(KEY_VALUE *pBuffer);
void Key_Value_Test(KEY_VALUE *pBuffer);

#endif