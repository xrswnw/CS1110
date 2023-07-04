#ifndef _ANYID_CS1110_KEY_HL_H
#define _ANYID_CS1110_KEY_HL_H

#include "AnyID_CS1110_Config.h"


extern const PORT_INF KEY_POW_PORT;
#define Key_Pow_LOW()                  KEY_POW_PORT.Port->BRR = KEY_POW_PORT.Pin
#define Key_Low_High()                 KEY_POW_PORT.Port->BSRR = KEY_POW_PORT.Pin

#define KEY_SAMPLE_UP           0x01
#define KEY_SAMPLE_MIDDLE       0x02
#define KEY_SAMPLE_DOWN         0x04
void Key_InitInterface();
u8 Key_GetValue();
#endif