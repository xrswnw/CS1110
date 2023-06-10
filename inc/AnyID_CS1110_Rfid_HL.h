#ifndef _ANYID_CS1110_RFID_HL_H
#define _ANYID_CS1110_RFID_HL_H

#include "AnyID_CS1110_Config.h"

#define RFID_PORT                      UART5
#define RFID_IRQHandler                UART5_IRQHandler
                                             
#define RFID_BAUDRARE                  38400

#define Rfid_ChkTxOver()                 while(((RFID_PORT)->SR & USART_FLAG_TC) == (u16)RESET)

#define Rfid_ReadByte()                  ((u16)(RFID_PORT->DR & (u16)0x01FF))



extern const PORT_INF RFID_PORT_CTRL;
#define Rfid_EnableRx()                     (RFID_PORT_CTRL.Port->BRR  = RFID_PORT_CTRL.Pin)
#define Rfid_EnableTx()                     (RFID_PORT_CTRL.Port->BSRR = RFID_PORT_CTRL.Pin)

void Rfid_InitInterface(u32 baudrate);
void Rfid_ConfigInt(void);
void Rfid_EnableInt(FunctionalState rxState, FunctionalState txState);

void Rfid_Delayms(u32 n);
void Rfid_Delay100us(u32 n);
void Rfid_WriteByte(u8 ch);
void Rfid_WriteBuffer(u8 *pFrame, u16 len);


#endif