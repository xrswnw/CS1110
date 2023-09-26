#ifndef _ANYID_CS1110_UART_HL_H
#define _ANYID_CS1110_UART_HL_H

#include "AnyID_CS1110_Config.h"

#define UART_PORT                       USART3
#define Uart_IRQHandler                 USART3_IRQHandler

extern const PORT_INF UART_PORT_POE;
#define UART_BAUDRARE                   115200

#define Uart_ReadByte()                 ((u16)(UART_PORT->DR & (u16)0x01FF))
void Uart_InitInterface(u32 baudrate);

void Uart_ConfigInt(void);
void Uart_EnableInt(FunctionalState rxState, FunctionalState txState);

void Uart_WriteByte(u8 ch);
void Uart_WriteBuffer(u8 *pFrame, u16 len);

void Uart_WriteCmd(char *str);
#endif
