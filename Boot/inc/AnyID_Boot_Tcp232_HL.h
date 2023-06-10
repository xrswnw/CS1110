#ifndef _ANYID_BOOT_TCP232_HL_
#define _ANYID_BOOT_TCP232_HL_

#include "AnyID_Boot_Config.h"

#define TCP232_PORT                       USART3
#define Tcp232_IRQHandler                 USART3_IRQHandler
#define TCP232_BAUDRARE                   115200

#define Tcp232_ReadByte()                 ((u16)(TCP232_PORT->DR & (u16)0x01FF))

void Tcp232_InitInterface(u32 baudrate);

void Tcp232_ConfigInt(void);
void Tcp232_EnableInt(FunctionalState rxState, FunctionalState txState);

void Tcp232_WriteByte(u8 ch);
void Tcp232_WriteBuffer(u8 *pFrame, u16 len);

#endif
