#ifndef _ANYID_CS1110_UART_H
#define _ANYID_CS1110_UART_H

#include "AnyID_CS1110_Uart_HL.h"
#include "AnyID_Uart_Receive.h"

extern UART_RCVFRAME g_sUartRcvTempFrame;
extern UART_RCVFRAME g_sRfidRcvTempFrame;
void Uart_Init();
#endif
