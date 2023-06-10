#ifndef _ANYID_CS1110_KEY_HL_H
#define _ANYID_CS1110_KEY_HL_H

#include "AnyID_CS1110_Config.h"


extern const PORT_INF KEY_POW_PORT;
#define Key_Pow_LOW()                  KEY_POW_PORT.Port->BRR = KEY_POW_PORT.Pin
#define Key_Low_High()                 KEY_POW_PORT.Port->BSRR = KEY_POW_PORT.Pin


void Key_InitInterface();
u8 Key_GetValue();
#endif