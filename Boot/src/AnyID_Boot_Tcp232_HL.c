#include "AnyID_Boot_Tcp232_HL.h"

#define TCP232_INT_CHANNEL        USART3_IRQn
const PORT_INF TCP232_PORT_TX = {GPIOB, GPIO_Pin_10};
const PORT_INF TCP232_PORT_RX = {GPIOB, GPIO_Pin_11};
void Tcp232_InitInterface(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    USART_DeInit(TCP232_PORT);

    GPIO_InitStructure.GPIO_Pin = TCP232_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(TCP232_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = TCP232_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(TCP232_PORT_RX.Port, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    //Configure USART
    USART_Init(TCP232_PORT, &USART_InitStructure);
    // Enable the USART
    USART_Cmd(TCP232_PORT, ENABLE);
}

void Tcp232_ConfigInt(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = TCP232_INT_CHANNEL;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART >> 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Tcp232_EnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(TCP232_PORT, USART_IT_RXNE | USART_IT_TC);

    USART_ITConfig(TCP232_PORT, USART_IT_RXNE, rxState);
    USART_ITConfig(TCP232_PORT, USART_IT_TC, txState);
}

void Tcp232_WriteByte(u8 ch)
{
    while(((TCP232_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(TCP232_PORT)->DR = (ch & (u16)0x01FF);
}

void Tcp232_WriteBuffer(u8 *pFrame, u16 len)
{
    u16 i = 0;
    for(i = 0; i < len; i++)
    {
        while(((TCP232_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
        (TCP232_PORT)->DR = (pFrame[i] & (u16)0x01FF);
    }
}
