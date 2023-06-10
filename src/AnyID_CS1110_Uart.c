#include "AnyID_CS1110_Uart.h"

UART_RCVFRAME g_sUartRcvTempFrame = {0};
UART_RCVFRAME g_sRfidRcvTempFrame = {0};
void Uart_Init()
{
    Uart_InitInterface(UART_BAUDRARE);
    Uart_ConfigInt();
    Uart_EnableInt(ENABLE, DISABLE);
}