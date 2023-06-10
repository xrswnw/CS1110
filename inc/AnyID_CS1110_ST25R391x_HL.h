#ifndef _ANYID_CS1110_ST25R3916_HL_H
#define _ANYID_CS1110_ST25R3916_HL_H

#include "AnyID_CS1110_Config.h"

extern const PORT_INF ST25R3916_CS_COM;
#define St25r3916_CsLow()           ST25R3916_CS_COM.Port->BRR = ST25R3916_CS_COM.Pin
#define St25r3916_CsHigh()          ST25R3916_CS_COM.Port->BSRR = ST25R3916_CS_COM.Pin

#define ST25R3916_PORT              SPI1
#define ST25R3916_SPI_DUMMY         0xFF

#define READER_RF_OPEN              0x01









void St25r3916_InitInterface(void);
void St25r3916_WriteByte(u8 v);
u8 St25r3916_ReadByte(void);
void St25r3916_Delayms(u32 n);
void St25r3916_Delay16us(u32 us);
#endif
