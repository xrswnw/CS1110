#include "AnyID_CS1110_Rfid_HL.h"

#define RFID_INT_CHANNEL           UART5_IRQn
const PORT_INF RFID_PORT_TX      = {GPIOC, GPIO_Pin_12};
const PORT_INF RFID_PORT_RX      = {GPIOD, GPIO_Pin_2};
const PORT_INF RFID_PORT_CTRL    = {GPIOB, GPIO_Pin_5};

void Rfid_InitInterface(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    USART_DeInit(RFID_PORT);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_InitStructure.GPIO_Pin = RFID_PORT_CTRL.Pin;
    GPIO_Init(RFID_PORT_CTRL.Port, &GPIO_InitStructure);
    Rfid_EnableRx();
    GPIO_InitStructure.GPIO_Pin = RFID_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(RFID_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = RFID_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(RFID_PORT_RX.Port, &GPIO_InitStructure);    
    
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    //Configure USART
    USART_Init(RFID_PORT, &USART_InitStructure);
    // Enable the USART
    USART_Cmd(RFID_PORT, ENABLE);
}

void Rfid_ConfigInt(void)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    NVIC_InitStructure.NVIC_IRQChannel = RFID_INT_CHANNEL;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_RX >> 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART_RX & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Rfid_EnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(RFID_PORT, USART_IT_RXNE | USART_IT_TC);

    USART_ITConfig(RFID_PORT, USART_IT_RXNE, rxState);
    USART_ITConfig(RFID_PORT, USART_IT_TC, txState);
}

void Rfid_WriteByte(u8 ch)
{
    while(((RFID_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(RFID_PORT)->DR = (ch & (u16)0x01FF);
}

void Rfid_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}

void Rfid_WriteBuffer(u8 *pFrame, u16 len)
{
    u16 i = 0;
    Rfid_EnableTx();
    for(i = 0; i < len; i++)
    {
        (RFID_PORT)->DR = (pFrame[i] & (u16)0x01FF);
        while(((RFID_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
    }
    Rfid_ChkTxOver();
    Rfid_EnableRx();
}


