#include "AnyID_CS1110_ST25R391x_HL.h"

const PORT_INF ST25R3916_CS_COM     = {GPIOA, GPIO_Pin_4};
const PORT_INF ST25R3916_PORT_SPI = {GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7};
void St25r3916_InitInterface(void)
{
    SPI_InitTypeDef   SPI_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //cs
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = ST25R3916_CS_COM.Pin;
    GPIO_Init(ST25R3916_CS_COM.Port, &GPIO_InitStructure);
    St25r3916_CsHigh();

    //SPI
    GPIO_InitStructure.GPIO_Pin = ST25R3916_PORT_SPI.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(ST25R3916_PORT_SPI.Port, &GPIO_InitStructure);

    SPI_Cmd(ST25R3916_PORT, DISABLE);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex ;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master ;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b ;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low ;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge ;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft ;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;  // 9MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB ;
    SPI_InitStructure.SPI_CRCPolynomial = 7 ;
    SPI_Init(ST25R3916_PORT, &SPI_InitStructure);

    SPI_Cmd(ST25R3916_PORT, ENABLE);
}

void St25r3916_WriteByte(u8 v)
{
    SPI_Cmd(ST25R3916_PORT, DISABLE);
    SPI_Cmd(ST25R3916_PORT, ENABLE);
    if(SPI_I2S_GetFlagStatus(ST25R3916_PORT, SPI_I2S_FLAG_RXNE) == SET)
    {
        SPI_I2S_ReceiveData(ST25R3916_PORT);
    }

    while(SPI_I2S_GetFlagStatus(ST25R3916_PORT, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(ST25R3916_PORT, v);
    while(SPI_I2S_GetFlagStatus(ST25R3916_PORT, SPI_I2S_FLAG_RXNE) == RESET);

    v = SPI_I2S_ReceiveData(ST25R3916_PORT);
}

u8 St25r3916_ReadByte(void)
{
    SPI_Cmd(ST25R3916_PORT, DISABLE);
    SPI_Cmd(ST25R3916_PORT, ENABLE);
    if(SPI_I2S_GetFlagStatus(ST25R3916_PORT, SPI_I2S_FLAG_RXNE) == SET)
    {
        SPI_I2S_ReceiveData(ST25R3916_PORT);
    }

    while(SPI_I2S_GetFlagStatus(ST25R3916_PORT, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(ST25R3916_PORT, ST25R3916_SPI_DUMMY);
    while(SPI_I2S_GetFlagStatus(ST25R3916_PORT, SPI_I2S_FLAG_RXNE) == RESET);

    return SPI_I2S_ReceiveData(ST25R3916_PORT);
}

void St25r3916_Delayms(u32 n)
{
    n *= 0x3800;        //72MHZ
    n++;
    while(n--);
}

void St25r3916_Delay16us(u32 us)
{
    u32 delay = 0;
    while(us--)
    {
        delay = 0x120;
        while(delay--);
    }
}
