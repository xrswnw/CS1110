#include "AnyID_CS1110_DishLcm_HL.h"

#define LCM_DISH_INT_CHANNEL           USART1_IRQn
const PORT_INF LCM_DISH_PORT_TX      = {GPIOA, GPIO_Pin_9};
const PORT_INF LCM_DISH_PORT_RX      = {GPIOA, GPIO_Pin_10};
void Lcm_DishInitInterface(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    USART_DeInit(LCM_DISH_PORT);

    GPIO_InitStructure.GPIO_Pin = LCM_DISH_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(LCM_DISH_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LCM_DISH_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(LCM_DISH_PORT_RX.Port, &GPIO_InitStructure);    
    
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    //Configure USART
    USART_Init(LCM_DISH_PORT, &USART_InitStructure);
    // Enable the USART
    USART_Cmd(LCM_DISH_PORT, ENABLE);
    USART_ITConfig(LCM_DISH_PORT, USART_IT_IDLE, ENABLE);
    USART_DMACmd(LCM_DISH_PORT, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(LCM_DISH_PORT, USART_DMAReq_Rx, ENABLE);
}

void Lcm_DishConfigInt(FunctionalState state)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    NVIC_InitStructure.NVIC_IRQChannel = LCM_DISH_INT_CHANNEL;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_RX >> 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART_RX & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;

    NVIC_Init(&NVIC_InitStructure);
    
    
}

void Lcm_DishEnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(LCM_DISH_PORT, USART_IT_RXNE | USART_IT_TC);

    USART_ITConfig(LCM_DISH_PORT, USART_IT_RXNE, rxState);
    USART_ITConfig(LCM_DISH_PORT, USART_IT_TC, txState);
}

void Lcm_DishWriteByte(u8 ch)
{
    while(((LCM_DISH_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(LCM_DISH_PORT)->DR = (ch & (u16)0x01FF);
}

void Lcm_DishDelayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}

void Lcm_DishWriteBuffer(u8 *pFrame, u16 len)
{
    u16 i = 0;
    Lcm_DishDelayms(1);
    for(i = 0; i < len; i++)
    {
		Lcm_DishWriteByte(pFrame[i]);
        /*(LCM_DISH_PORT)->DR = (pFrame[i] & (u16)0x01FF);
        while(((LCM_DISH_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);  */     //删除导致数据发送不完整
    }
}


void Lcm_InitTxDma(u8 *pTxBuffer, u32 len)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    //DMA1通道5配置
    DMA_DeInit(LCM_TXDMA_CH);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(LCM_DISH_PORT->DR));
    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)pTxBuffer;
    //dma传输方向单向
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    //设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_BufferSize = len;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    //设置DMA的传输模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(LCM_TXDMA_CH, &DMA_InitStructure);

    DMA_ITConfig(LCM_TXDMA_CH, DMA_IT_TC, ENABLE);  //open DMA send inttrupt
    Lcm_DisableTxDma();// 关闭DMA
    
    //dma interrupt
    NVIC_InitStructure.NVIC_IRQChannel = LCM_TXDMA_INT;                                       //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_TXDMA >> 2;    //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART_TXDMA & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Lcm_InitRxDma(u8 *pRxBuffer, u32 len)
{
    NVIC_InitTypeDef  NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};


    DMA_DeInit(LCM_RXDMA_CH);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(LCM_DISH_PORT->DR));
    
    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)pRxBuffer;
    //dma传输方向单向
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    //设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_BufferSize = len;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    //设置DMA的传输模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    
    DMA_Init(LCM_RXDMA_CH, &DMA_InitStructure);
    DMA_ITConfig(LCM_RXDMA_CH, DMA_IT_TC, ENABLE);  //open DMA send inttrupt
    Lcm_DisableRxDma();// 关闭DMA
    
    //dma interrupt
    NVIC_InitStructure.NVIC_IRQChannel = LCM_RXDMA_INT;   

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_RXDMA >> 2;    //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART_RXDMA & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Lcm_Stop(void)
{
    Lcm_DisableRxDma();
    Lcm_DisableTxDma();
    Lcm_DishConfigInt(DISABLE);

    USART_ITConfig(LCM_DISH_PORT, USART_IT_IDLE, DISABLE);
    USART_DMACmd(LCM_DISH_PORT, USART_DMAReq_Tx, DISABLE);
    USART_DMACmd(LCM_DISH_PORT, USART_DMAReq_Rx, DISABLE);
    USART_Cmd(LCM_DISH_PORT, DISABLE);
    USART_DeInit(LCM_DISH_PORT);
}


void Lcm_WriteByte(u8 ch)
{
    //Lcm_Delayms(1);
    (LCM_DISH_PORT)->DR = (ch & (u16)0x01FF);
    while(((LCM_DISH_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
}

void Lcm_Delayms(u32 n)             //系统延时n毫秒
{
    n *= 0x6000;
    n++;
    while(n--);
}




