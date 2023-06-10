#include "AnyID_CS1110_DishLcm.h"

LCM_DISH_TXFRAME g_sLcmDishTxFrame = {0};
LCM_DISH_RCVFRAME g_sLcmDishRcvFrame = {0};
LCM_DISH_RCVFRAME g_sLcmDishRcvTempFrame = {0};
LCM_INFO g_sLcmInfo = {0};

void Lcm_Init()
{
    memset(&g_sLcmDishRcvTempFrame, 0, sizeof(LCM_DISH_RCVFRAME));
    memset(&g_sLcmDishRcvFrame, 0, sizeof(LCM_DISH_RCVFRAME));
    memset(&g_sLcmDishTxFrame, 0, sizeof(LCM_DISH_TXFRAME));
    memset(&g_sLcmInfo, 0, sizeof(LCM_INFO));
    Lcm_DishInitInterface(LCM_DISH_BAUDRARE);
    Lcm_DishConfigInt(ENABLE);
    Lcm_InitTxDma(g_sLcmDishTxFrame.frame, LCM_BUFFER_MAX_LEN);
    Lcm_InitRxDma(g_sLcmDishRcvFrame.buffer, LCM_BUFFER_MAX_LEN);
    Lcm_EnableRxDma();

}


void Lcm_DishReceiveFrame(u8 byte, LCM_DISH_RCVFRAME *pRcvFrame)
{
   
}

u8 Lcm_DishWriteRegFrame(u8 addr, u8 *pReg, u8 num, u8 *pFrame)
{
    u8 pos = 0;
    u8 i = 0;

    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD1;
    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD2;

    pFrame[pos++] = 0xFF;                                       
    pFrame[pos++] = LCM_DISH_FRAME_CMD_WREG;                              

    pFrame[pos++] = addr;                                                      
    for(i = 0; i < num; i++)            
    {
        pFrame[pos++] = pReg[i];                                                  
    }

    pFrame[LCM_DISH_FRAME_POS_LEN] = pos - 3;

    return pos;
}

u8 Lcm_DishWriteVarFrame(u16 addr, u8 *pVar, u16 varLen, u8 *pFrame)
{
    u8 pos = 0;
    u8 i = 0;

    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD1;
    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD2;

    pFrame[pos++] = 0xFF;                                                       //数据长度，从命令码开始计算
    pFrame[pos++] = LCM_DISH_FRAME_CMD_WVAR;                                    //写变量地址内容命令

    pFrame[pos++] = (addr >> 8) & 0xFF;                                         //高地址在前
    pFrame[pos++] = (addr >> 0) & 0xFF;
    for(i = 0; i < varLen; i++)
    {
        pFrame[pos++] = pVar[i];
    }

    pFrame[LCM_DISH_FRAME_POS_LEN] = pos - 3;

    return pos;
}


u8 Lcm_DishWriteVarUniCodeFrame(u16 addr, u8 *pVar, u16 varLen, u8 *pFrame)
{
    u8 pos = 0;
    u8 i = 0;

    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD1;
    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD2;

    pFrame[pos++] = 0xFF;                                            
    pFrame[pos++] = LCM_DISH_FRAME_CMD_WVAR;                             

    pFrame[pos++] = (addr >> 8) & 0xFF;                           
    pFrame[pos++] = (addr >> 0) & 0xFF;
    for(i = 0; i < varLen; i++)
    {
        pFrame[pos++] = 0x00;
        pFrame[pos++] = pVar[i];
    }

    pFrame[LCM_DISH_FRAME_POS_LEN] = pos - 3;

    return pos;
}


void Lcm_DishSelectPage(u16 page)
{
    u8 reg[2] = {0};
    reg[0] = (page >> 8) & 0xFF;                           //高地址在前
    reg[1] = (page >> 0) & 0xFF;
    g_sLcmDishTxFrame.len = Lcm_DishWriteRegFrame(LCM_DISH_REG_ADDR_PICID, reg, 2, g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}

void Lcm_SelectPage(u16 page)
{
    g_sLcmDishTxFrame.len = Lcm_ChgPageFormatFrame(page,g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}

void Lcm_SelectIco(u16 ico,u16 addr)
{
    g_sLcmDishTxFrame.len = Lcm_FormatFrame(ico, addr, g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}


void Lcm_DishWriteColor(u16 addr, u16 color)
{
    g_sLcmDishTxFrame.len = Lcm_DishWriteVarFrame(addr + 0x03, (u8 *)(&color), 2, g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}

void Lcm_DishWriteTxt(u16 addr, u8 bufLen, char *pTxt, u8 mode, u8 offset)
{
    char txt[LCM_DISH_TXT_LEN_MAX] = {0};
    u8 txtLen = 0;
    u8 pos = 0;

    txtLen = strlen(pTxt);
    if(txtLen > bufLen)
    {
        txtLen = bufLen;
        memcpy(txt, pTxt, txtLen);
    }
    else
    {
        memset(txt, ' ', LCM_DISH_TXT_LEN_MAX);

        if(mode == LCM_DISH_TX_MODE_MID)
        {
            if(txtLen & 0x01)
            {
                pos = (bufLen - (txtLen - 1)) >> 1;
            }
            else
            {
                pos = (bufLen - txtLen) >> 1;
            }
            
        }
        else if(mode == LCM_DISH_TX_MODE_LEFT)
        {
            pos = offset;
        }
        else if(mode == LCM_DISH_TX_MODE_RIGHT)
        {
            pos = bufLen - offset - txtLen;
        }
        memcpy(txt + pos, pTxt, txtLen);
    }

    g_sLcmDishTxFrame.len = Lcm_DishWriteVarFrame(addr, (u8 *)txt, bufLen, g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}


void Lcm_DishWriteUniCodeTxt(u16 addr, u8 bufLen, char *pTxt, u8 mode, u8 offset)   //UNICODE编码，自定义字符
{
    char txt[LCM_DISH_TXT_LEN_MAX] = {0};
    u8 txtLen = 0;
    u8 pos = 0;

    txtLen = strlen(pTxt);
    if(txtLen > bufLen)
    {
        txtLen = bufLen;
        memcpy(txt, pTxt, txtLen);
    }
    else
    {
        memset(txt, ' ', LCM_DISH_TXT_LEN_MAX);

        if(mode == LCM_DISH_TX_MODE_MID)
        {
            if(txtLen & 0x01)
            {
                pos = (bufLen - (txtLen - 1)) >> 1;
            }
            else
            {
                pos = (bufLen - txtLen) >> 1;
            }
            
        }
        else if(mode == LCM_DISH_TX_MODE_LEFT)
        {
            pos = offset;
        }
        else if(mode == LCM_DISH_TX_MODE_RIGHT)
        {
            pos = bufLen - offset - txtLen;
        }
        memcpy(txt + pos, pTxt, txtLen);
    }

    g_sLcmDishTxFrame.len = Lcm_DishWriteVarUniCodeFrame(addr, (u8 *)txt, bufLen, g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}


u8 Lcm_ChgPageFormatFrame(u16 page,u8 *pFrame)
{
    u8 pos = 0;

    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD1;
    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD2;

    pFrame[pos++] = 0xFF;                                          
    pFrame[pos++] = LCM_DISH_FRAME_CMD_WVAR;                       
    pFrame[pos++] = (LCM_CHG_PAGE_ADDR >> 8) & 0xFF;                           
    pFrame[pos++] = (LCM_CHG_PAGE_ADDR >> 0) & 0xFF;                           
    pFrame[pos++] = (LCM_CHG_PAGE_FIX_FRAME >> 8) & 0xFF ;                          
    pFrame[pos++] = (LCM_CHG_PAGE_FIX_FRAME >> 0) & 0xFF;                           
    pFrame[pos++] = (page >> 8) & 0xFF ;                         
    pFrame[pos++] = (page >> 0) & 0xFF ;                                              
    pFrame[LCM_DISH_FRAME_POS_LEN] = pos - 3;

    return pos;
}

u8 Lcm_FormatFrame(u16 data,u16 addr,u8 *pFrame)
{
    u8 pos = 0;

    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD1;
    pFrame[pos++] = LCM_DISH_FRAME_FLAG_HEAD2;

    pFrame[pos++] = 0xFF;                                          
    pFrame[pos++] = LCM_DISH_FRAME_CMD_WVAR;                       
    pFrame[pos++] = (addr >> 8) & 0xFF;                           
    pFrame[pos++] = (addr >> 0) & 0xFF;                                                     
    pFrame[pos++] = (data >> 8) & 0xFF ;                         
    pFrame[pos++] = (data >> 0) & 0xFF ;                                              
    pFrame[LCM_DISH_FRAME_POS_LEN] = pos - 3;

    return pos;
}

void Lcm_ClearMain()
{
    char asciiBuf[LCM_TXT_LEN_MAX] = ""; 
    Lcm_DishWriteTxt(LCM_PAGE_MAIN_WHITE, LCM_TXT_LEN_MAX, asciiBuf, LCM_DISH_TX_MODE_LEFT, 0);
    Lcm_DishWriteTxt(LCM_PAGE_MAIN_WHITE, LCM_TXT_LEN_MAX, asciiBuf, LCM_DISH_TX_MODE_LEFT, 0);
}

void Lcm_BackgroundCtr(u16 data,u16 addr)
{
    g_sLcmDishTxFrame.len = Lcm_FormatFrame(data, addr, g_sLcmDishTxFrame.frame);
    Lcm_DishWriteBuffer(g_sLcmDishTxFrame.frame, g_sLcmDishTxFrame.len);
}



