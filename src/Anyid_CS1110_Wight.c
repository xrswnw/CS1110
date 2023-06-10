#include "AnyID_CS1110_Wight.h"

GPB_INFO g_sGpbInfo = {0};
GPB_RX_BUF g_sGpbTempRcv = {0};

void GPB_Init()
{	
    GPB_InitInterface(GPB_BAUDRARE);
    GPB_ConfigInt(ENABLE);
    GPB_InitTxDma(g_sGpbInfo.txBuf.buffer, g_sGpbInfo.txBuf.len);
    GPB_InitRxDma( g_sGpbInfo.rxBuf.buffer, g_sGpbInfo.rxBuf.len);
    GPB_EnableRxDma();
    g_sGpbInfo.state =GPB_STAT_IDLE;
    g_sGpbInfo.witghSmple = GPB_STAT_OPEN_SAMPLE;
    g_sGpbInfo.txBuf.cmd = GPB_MODBUS_READ_REG;
    g_sGpbInfo.txBuf.regAdd = GPB_REG_ADDR_WGT;
    g_sGpbInfo.txBuf.regNum = 0x0002;
}

u8 GPB_RFormatFrame(u8 cmd,u16 regAdd,u16 regNum,u8 *pFrame)
{
    u8 pos = 0;
    u16 crc = 0;
    pFrame[pos++] = 0xFF;
    pFrame[pos++] = cmd;
    pFrame[pos++] = (regAdd >> 8) & 0xFF;
    pFrame[pos++] = (regAdd >> 0) & 0xFF;
    pFrame[pos++] = (regNum >> 8) & 0xFF;
    pFrame[pos++] = (regNum >> 0) & 0xFF;
	//校验
    crc = GPB_GetCrc16(pFrame, pos);
    pFrame[pos++] = (crc >> 0) & 0xFF;
    pFrame[pos++] = (crc >> 8) & 0xFF;
    return pos;
}

u8 GPB_WFormatFrame(u8 cmd,u8 len,u16 regAdd,u16 regNum,u8 *pFrame, u16 data0,u16 data1)//写两个寄存器
{
    u8 pos = 0;
    u16 crc = 0;
    pFrame[pos++] = 0xFF;
    pFrame[pos++] = cmd;
    pFrame[pos++] = (regAdd >> 8) & 0xFF;
    pFrame[pos++] = (regAdd >> 0) & 0xFF;
    pFrame[pos++] = (regNum >> 8) & 0xFF;
    pFrame[pos++] = (regNum >> 0) & 0xFF;
    pFrame[pos++] = (len >> 0) & 0xFF;
    pFrame[pos++] = (data0 >> 8) & 0xFF;
    pFrame[pos++] = (data0 >> 0) & 0xFF;
    pFrame[pos++] = (data1 >> 8) & 0xFF;
    pFrame[pos++] = (data1 >> 0) & 0xFF;
	//校验
    crc = GPB_GetCrc16(pFrame, pos);
    pFrame[pos++] = (crc >> 0) & 0xFF;
    pFrame[pos++] = (crc >> 8) & 0xFF;
    return pos;
}


#define POLYNOMIAL                      0xA001   //x^16 + x^12 + x^5 + 1
#define PRESET_VALUE                    0xFFFF
u16 GPB_GetCrc16(u8 *pBuffer, u8 len)
{
    u16 i = 0;
    u16 crc = 0;
    u8 j = 0;

    crc = PRESET_VALUE;
    for(i = 0; i < len; i++)
    {
        crc = crc ^ pBuffer[i];
        for(j = 0; j < 8; j++)
        {
            if(crc & 0x0001)
            {
                crc = (crc >> 1) ^ POLYNOMIAL;
            }
            else
            {
                crc = (crc >> 1);
            }
        }
    }

    return crc;
}


BOOL GPB_CheckFrame(GPB_RX_BUF *pRcvFrame)
{
    BOOL b = FALSE;

    if(pRcvFrame->len >= GPB_BUF_MIN_LEN)
    {
        u16 crc1 = 0, crc2 = 0;
        crc1 = GPB_GetCrc16(pRcvFrame->buffer, pRcvFrame->len - 2);
        crc2 = pRcvFrame->buffer[pRcvFrame->len - 1] ;
        crc2 <<= 8;
        crc2 |= pRcvFrame->buffer[pRcvFrame->len - 2];		
        if(crc1 == crc2)
        {
            b = TRUE;
        }
    }
    return b;
}


void GPB_TransmitCmd(GPB_TX_BUF *pBuffer)
{	
    g_sGpbInfo.state =GPB_STAT_IDLE;
    g_sGpbInfo.txBuf.cmd = GPB_MODBUS_READ_REG;
    g_sGpbInfo.txBuf.regAdd = GPB_REG_ADDR_WGT;
    g_sGpbInfo.txBuf.regNum = 0x0002;
    g_sGpbInfo.txBuf.len = GPB_RFormatFrame(pBuffer->cmd, pBuffer->regAdd, pBuffer->regNum,  pBuffer->buffer);
    GPB_WriteBuffer(g_sGpbInfo.txBuf.buffer, g_sGpbInfo.txBuf.len);
}

void GPB_Adjust(GPB_TX_BUF *pBuffer)
{
    pBuffer->cmd = GPB_MODBUS_SET_REG;
    pBuffer->regAdd = GPB_REG_ADDR_WGT;
    pBuffer->regNum = 0x0002;
    pBuffer->data0 = 0x0000;
    pBuffer->data1 = 0x0001;
    g_sGpbInfo.txBuf.len = GPB_WFormatFrame(pBuffer->cmd, 4, pBuffer->regAdd, pBuffer->regNum, pBuffer->buffer, pBuffer->data0, pBuffer->data1);
    
    GPB_WriteBuffer(g_sGpbInfo.txBuf.buffer, g_sGpbInfo.txBuf.len);

}

void GPB_RtoChg(GPB_TX_BUF *pBuffer)
{
    pBuffer->cmd = GPB_MODBUS_READ_REG;
    pBuffer->regAdd = GPB_REG_ADDR_RTO;
    pBuffer->regNum = 0x0002;
    g_sGpbInfo.txBuf.len = GPB_WFormatFrame(pBuffer->cmd, 4, pBuffer->regAdd, pBuffer->regNum, pBuffer->buffer, pBuffer->data0, pBuffer->data1);
    
    GPB_WriteBuffer(g_sGpbInfo.txBuf.buffer, g_sGpbInfo.txBuf.len);

}



void GPB_Filt_Chg()
{
    u8 pos = 0;
    u16 crc;
    g_sGpbInfo.txBuf.buffer[pos++] = 0xFF;
    g_sGpbInfo.txBuf.buffer[pos++] = GPB_MODBUS_SET_REG;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x00;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x0D;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x00;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x02;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x04;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x00;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x00;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x00;
    g_sGpbInfo.txBuf.buffer[pos++] = 0x03;
    
    crc = GPB_GetCrc16(g_sGpbInfo.txBuf.buffer, pos);
    g_sGpbInfo.txBuf.buffer[pos++] = (crc >> 0) & 0xFF;
    g_sGpbInfo.txBuf.buffer[pos++] = (crc >> 8) & 0xFF;
    g_sGpbInfo.txBuf.len = pos;
     GPB_WriteBuffer(g_sGpbInfo.txBuf.buffer, g_sGpbInfo.txBuf.len);

}


void GPB_ChgValue(u32 value, char *buf)
{
    u32 tempValue = value & 0x000FFFFF;
    if(tempValue < 10000)
    {
         sprintf(buf, "%d.%d%d%dKg", (value & 0x000FFFFF) / 1000, ((value & 0x000FFFFF) % 1000) / 100, ((value & 0x000FFFFF) % 100) / 10, ((value & 0x000FFFFF) % 100) % 10);    
    }
    else if( 10000 <= tempValue < 100000)
    {
        sprintf(buf, "%d%d.%d%d%dKg", (value & 0x000FFFFF) / 10000, ((value %10000) & 0x000FFFFF) / 1000, ((value & 0x000FFFFF) % 1000) / 100, ((value & 0x000FFFFF) % 100) / 10, ((value & 0x000FFFFF) % 100) % 10); 
    }
    else if( 100000 <= tempValue < 1000000)
    {
        sprintf(buf, "%d%d%d.%d%d%dKg", (value & 0x000FFFFF) / 100000, (value & 0x000FFFFF) / 1000, (value & 0x000FFFFF) / 1000, ((value & 0x000FFFFF) % 1000) / 100, ((value & 0x000FFFFF) % 100) / 10, ((value & 0x000FFFFF) % 100) % 10); 
    }
}


u32 GPB_GetWightValue(u32 sampleValue)
{
    u32 value;
    u32 temp = 0;
    temp = (sampleValue & 0x00F00000) >> 20;
    value = ((sampleValue & 0x000FFFFF) / pow(10,temp)) * 1000;
    if(sampleValue & 0x10000000)
    {
      value = -value;
    }
    return value;
}

