#include "AnyID_CS1110_Reader.h"

const u8 READER_VERSION[READER_VERSION_SIZE]@0x08005000 = " CS1110 23061000 G230200";

DISH_INFO g_sDishTempInfo = {0};
READER_DEVICE_PARAMETER g_sDeviceParamenter = {0};
READER_INFO g_sRaderInfo ={0};
READER_RFID_INFO g_sReaderRfidTempInfo= {0};
READER_RSPFRAME g_sDeviceRspFrame = {0};
READER_IMPARAMS g_sReaderImParams = {0};
READER_OFFLINE_INFO g_sReaderOffLineInfo = {0};
READER_LED_INFO g_sReaderLedInfo = {0};

BOOL g_nReaderGpbState = FALSE;

u8 g_aReaderISO15693Uid[READER_OP_UID_MAX_NUM * ISO15693_SIZE_UID] = {0};
u8 g_nTempLink = 0;
u32  g_nTickLink = 0;

char g_nBufTxt[LCM_TXT_LEN_MAX] = "未绑定托盘"; 
char g_nBufTxt1[LCM_TXT_LEN_MAX] = ""; 
char g_aReaderVersion[8] = "23061000";

void Reader_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}


BOOL Reader_Rfid_Init()
{
    BOOL bOk = FALSE;
    St25r3916_InitInterface();
    bOk =  St25r3916_Init();
    ISO15693_Init();
    if(g_sDeviceParamenter.rfCtrl == READER_RF_OPEN)
    {
        St25r3916_OpenAntenna();
        Reader_Delayms(2);
    }
    else
    {
        St25r3916_CloseAntenna();
    }
    return bOk;

}

void Reader_Init()
{
    memset(&g_sRaderInfo, 0, sizeof(READER_INFO));
    memset(&g_sReaderRfidTempInfo, 0, sizeof(READER_RFID_INFO));
    memset(&g_sDeviceRspFrame, 0, sizeof(READER_RSPFRAME));

    if(Reader_ReadDeviceParamenter())
    {
        g_sLcmInfo.flag =  LCM_FLAG_PAGE_NULL_CHG;
    }
    else
    {
        g_sLcmInfo.flag = LCM_FLAG_PAGE_LINK_CHG;
     }
    //g_sDeviceParamenter.uiMode = READER_UI_MODE_WHITE;
    g_sDeviceParamenter.reWorkMode = READER_MODE_NORMAL;
    Reader_Font_ChgCorol(g_sDeviceParamenter.uiMode);
}



BOOL Reader_ReadDeviceParamenter(void)                                         //OK
{
    BOOL b = FALSE, bBackup = FALSE;
    BOOL bOk = FALSE;

    b = FRam_ReadBuffer(FRAM_SYS_INFO_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
    if(b)
    {
        u16 crc1 = 0, crc2 = 0;

        crc1 = a_GetCrc((u8 *)(&g_sDeviceParamenter), (sizeof(READER_DEVICE_PARAMETER)) - 4);
        crc2 = g_sDeviceParamenter.crc;

        //检测参数是否正确，如果不正确，使用默认参数操作
        if(crc1 != crc2)
        {
            b = FALSE;
        }
    }
    
    bBackup = FRam_ReadBuffer(FRAM_SYS_INFO_BACKUP_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
    if(bBackup)
    {
        u16 crc1 = 0, crc2 = 0;

        crc1 = a_GetCrc((u8 *)(&g_sDeviceParamenter), (sizeof(READER_DEVICE_PARAMETER)) - 4);
        crc2 = g_sDeviceParamenter.crc;

        //检测参数是否正确，如果不正确，使用默认参数操作
        if(crc1 != crc2)
        {
            bBackup = FALSE;
        }
    }

    if(b == FALSE && bBackup == FALSE)
    {
        memset(&g_sDeviceParamenter, 0, sizeof(g_sDeviceParamenter));
        g_sDeviceParamenter.reWorkMode = READER_MODE_NORMAL;
        g_sDeviceParamenter.rfCtrl = RFID_ANT_OPEN;
        g_sDeviceParamenter.addr = 0x0001;
        g_sDeviceParamenter.ip[0] = 192;
        g_sDeviceParamenter.ip[1] = 168;
        g_sDeviceParamenter.ip[2] = 1;
        g_sDeviceParamenter.ip[3] = 22;
        g_sDeviceParamenter.rfWorkMode = READER_RFID_READ_UID;
        g_sDeviceParamenter.totalMode = READER_TOTAL_MODE_AIR;
        g_sDeviceParamenter.uiMode = READER_UI_MODE_WHITE;
        g_sDeviceParamenter.scoWt = 50;
        Reader_WriteDeviceParamenter();
    }
     else if(b == TRUE && bBackup == FALSE)
    {
        FRam_ReadBuffer(FRAM_SYS_INFO_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
        FRam_WriteBuffer(FRAM_SYS_INFO_BACKUP_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
    }
    else if(b == FALSE && bBackup == TRUE)
    {
        FRam_ReadBuffer(FRAM_SYS_INFO_BACKUP_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
        FRam_WriteBuffer(FRAM_SYS_INFO_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
    }
    
    Fram_ReadBootParams();
    
    if((g_sFramBootParams.appState != FRAM_BOOT_APP_OK) || (g_sFramBootParams.br != g_sDeviceParamenter.bud) || (g_sFramBootParams.addr != g_sDeviceParamenter.addr))
    {
        g_sFramBootParams.appState = FRAM_BOOT_APP_OK;
        g_sFramBootParams.addr = g_sDeviceParamenter.addr;
        Fram_WriteBootParams();
        Fram_WriteBackupBootParams();
    }
    if(b == TRUE && bBackup == TRUE)
    {
        bOk =  TRUE;;
    }

    return bOk;
    

}


BOOL Reader_ReadOffLineDataNum(void)                                         //OK
{
    BOOL b = FALSE ;
    
    b = FRam_ReadBuffer(FRAM_OFF_LINE_NUM_ADDR, sizeof(READER_OFFLINE_INFO), (u8 *)(&g_sReaderOffLineInfo));
 
    return b;
    
}

BOOL Reader_ReadOffLineDatas(u16 addr, u16 size, u8 *pBuffer)                                      //OK
{
    BOOL b = FALSE ;
    
    b = FRam_ReadBuffer(addr, size, pBuffer);
     return b;
    
}


BOOL Reader_WriteDeviceParamenter(void)
{
    BOOL b = FALSE;

    g_sDeviceParamenter.crc = 0;
    g_sDeviceParamenter.crc = a_GetCrc((u8 *)(&g_sDeviceParamenter), (sizeof(READER_DEVICE_PARAMETER)) - 4);

    b = FRam_WriteBuffer(FRAM_SYS_INFO_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));
    b = FRam_WriteBuffer(FRAM_SYS_INFO_BACKUP_ADDR, sizeof(READER_DEVICE_PARAMETER), (u8 *)(&g_sDeviceParamenter));

    return b;
}



BOOL Reader_WriteOffLineDataNum(void)
{
    BOOL b = FALSE;
    g_sReaderOffLineInfo.crc = 0;
    g_sReaderOffLineInfo.crc = a_GetCrc((u8 *)(&g_sReaderOffLineInfo), (sizeof(READER_OFFLINE_INFO)) - 4);
    
    b = FRam_WriteBuffer(FRAM_OFF_LINE_NUM_ADDR, sizeof(READER_OFFLINE_INFO), (u8 *)(&g_sReaderOffLineInfo));
    return b;
}

BOOL Reader_WriteOffLineDatas(u16 addr, u16 size, u8 *pBuffer)
{
    BOOL b = FALSE;

    
    b = FRam_WriteBuffer(addr, size, pBuffer);
    return b;
}



void Fram_Demo()
{

    g_sFramData.id = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);
    g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
    Reader_ReadOffLineDatas(g_sFramData.id * FLASE_PAGE_SIZE + FRAM_OFF_LINE_ADDR, FLASH_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
    

}

u16 Reader_Seek_Id(u8 *pBuffer)          //扫描离线数据
{
    u8 tempId = 0x80;
    u16 i = 0;
    u8 j = 0;
    u16 id = 0;
    for(i = 0; i < READER_OFF_LINE_DATA_NUM; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if(pBuffer[i] & tempId)
            {
                return id;
                          //可取id
            }
            tempId >>= 1;
            id ++;
            if(!tempId)
            {
                tempId = 0x80;
            }
            
        }
    }
    return 0;
}

u16 Reader_Seek_Num(u8 *pBuffer)          //扫描离线数据
{
    u8 tempId = 0x80;
    u16 num = 0;
    u16 i = 0;
    u8 j = 0;
    for(i = 0; i < READER_OFF_LINE_DATA_NUM; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if(pBuffer[i] & tempId)
            {
                num ++;                           
            }
            tempId >>= 1;

            if(!tempId)
            {
                tempId = 0x80;
            }
            
        }
    }
    return num;
}

u16 Reader_Seek_Inq(u8 *pBuffer)          //扫描离线数据
{
    u8 tempId = 0x80;
    u16 i = 0;
    u8 j = 0;
    u16 id = 0;
    for(i = 0; i < READER_OFF_LINE_DATA_NUM; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if((pBuffer[i] & tempId) == 0)
            {
                return id;  //可取id
            }
            tempId >>= 1;
            id ++;
            if(!tempId)
            {
                tempId = 0x80;
            }
            
        }
    }
    return 0;
}
                
void Reader_DisplayIp(READER_DEVICE_PARAMETER *pBuffer)
{
    char asciiBuf[LCM_TXT_LEN_MAX] = ""; 
    sprintf(asciiBuf, "此设备IP地址为[%d.%d.%d.%d]", pBuffer->ip[0], pBuffer->ip[1], pBuffer->ip[2], pBuffer->ip[3]);    
    Lcm_DishWriteTxt(LCM_TXT_ADDR_LINK, 30, asciiBuf, LCM_DISH_TX_MODE_MID, 0);
    sprintf(asciiBuf, "%d.%d.%d.%d", pBuffer->ip[0], pBuffer->ip[1], pBuffer->ip[2], pBuffer->ip[3]); 
    Lcm_DishWriteTxt(LCM_TXT_ADDR_IP, 20, asciiBuf, LCM_DISH_TX_MODE_MID, 0);
}


/*
u8 Reader_RfidGetValue(u8 mode, READER_RFID_INFO *pRfidInfo)
{
    u8 state = 0;
    u8 i = 0;
    u8 tempUid[READER_UID_MAX_NUM * ISO15693_SIZE_UID] = {0};
    
    St25r3916_OpenAntenna();
    Reader_Delayms(4);

    
    if(mode == READER_RFID_READ_UID)
    {    
        pRfidInfo->tick++;
        state = ISO15693_Inventory(0, 0, tempUid);
        if(state == ISO15693_ERR_SUC)
        {
            pRfidInfo->okTick++;
            pRfidInfo->errTick = 0;
            if(pRfidInfo->okTick == 2)
            {
                pRfidInfo->state = RFID_TAG_IN;
                memcpy(pRfidInfo->uid, tempUid, ISO15693_SIZE_UID);
            }
            else if((pRfidInfo->okTick >2))
            {
                 pRfidInfo->state = RFID_TAG_KEEP;
                for(i = 0 ; i < ISO15693_SIZE_UID; i++)
                {
                    if(pRfidInfo->uid[i] != tempUid[i])
                    {
                        pRfidInfo->okTick = 0;
                        pRfidInfo->errTick += 15;
                        pRfidInfo->state = RFID_TAG_OUT;
                        break;
                    }
                }
            }
        }
        else
        {
            pRfidInfo->errTick ++;
            if(pRfidInfo->errTick >= 15)                                      //餐盘离开后1.5s上报数据
            {  
                if(g_sDeviceParamenter.reWorkMode == READER_MODE_NORMAL)
                {
                  pRfidInfo->okTick = 0;
                }
                pRfidInfo->state = RFID_TAG_OUT;
            }
        }
    }
    St25r3916_CloseAntenna(); 
    return state;
}


*/


u8 Reader_RfidGetValue(u8 *pBuffer, READER_RFID_INFO *pRfidInfo)
{
    u8 state = 0;
    u8 i = 0;
    u8 tempUid[READER_UID_MAX_NUM * ISO15693_SIZE_UID] = {0};
    u8 num = 0;
    num = *(pBuffer + 19 );
    pRfidInfo->tick++;
    pRfidInfo->num = num;
    if(num)
    {
        if(num == 0x01)
        {
            pRfidInfo->okTick++;
            pRfidInfo->errTick = 0;
            memcpy(tempUid, pBuffer + 19 + 1 , ISO15693_SIZE_UID);
            if(pRfidInfo->okTick == 2)
            {
                pRfidInfo->state = RFID_TAG_IN;
                memcpy(pRfidInfo->uid, tempUid, ISO15693_SIZE_UID);
            }
            else if((pRfidInfo->okTick >2))
            {
                 pRfidInfo->state = RFID_TAG_KEEP;
                for(i = 0 ; i < ISO15693_SIZE_UID; i++)
                {
                    if(pRfidInfo->uid[i] != tempUid[i])
                    {
                        pRfidInfo->okTick = 0;
                        pRfidInfo->errTick += READER_RFID_MOVE_TIME;
                        pRfidInfo->state = RFID_TAG_OUT;
                        break;
                    }
                }
            }
        }
        else
        {
            //多餐盘
          
            pRfidInfo->okTick = 0;
            pRfidInfo->errTick += READER_RFID_MOVE_TIME;
            pRfidInfo->state = RFID_TAG_OUT;
            pRfidInfo->clash = RFID_TAG_FAIL;
        
            
        }

    }
    else
    {
        pRfidInfo->errTick ++;
        if(pRfidInfo->errTick >= READER_RFID_MOVE_TIME)                                      //餐盘离开后1.5s上报数据
        {  
            if(g_sDeviceParamenter.reWorkMode == READER_MODE_NORMAL)
            {
                pRfidInfo->okTick = 0;
            }
                pRfidInfo->state = RFID_TAG_OUT;
        }
    }

    return state;
}

u8 Reader_DisplayTest(DISH_INFO *pDishInfo, GPB_INFO *gpbInfo)                                                                  
{
    char Buf[LCM_TXT_LEN_MAX] = ""; 
    u16 percentage = 0;
    u8 state = 0;
    
    percentage = ((float)g_sReaderRfidTempInfo.okTick / g_sReaderRfidTempInfo.tick )* 10000;
    //GPB_ChgValue(gpbInfo->wightTemp[0],Buf);
    if(gpbInfo->wightTemp & 0x10000000)
    {
        sprintf(Buf, "%dg", 0 - Sound_GetValue(g_sGpbInfo.wightTemp));
    }
    else
    {
        sprintf(Buf, "%dg", Sound_GetValue(g_sGpbInfo.wightTemp)); 
    }
     
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_1, 8, Buf, LCM_DISH_TX_MODE_LEFT, 0);
    sprintf(Buf, "%d个", g_sReaderRfidTempInfo.num);   
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_BUTTON_2, 3, Buf, LCM_DISH_TX_MODE_MID, 0);
    sprintf(Buf, "%d", g_sReaderRfidTempInfo.tick );    
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_2, 6, Buf, LCM_DISH_TX_MODE_LEFT, 0);
    sprintf(Buf, "%d%d.%d%d%", percentage / 1000 ,(percentage % 1000) / 100, (percentage % 100) / 10, (percentage % 100) % 10 );
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_3, 6, Buf, LCM_DISH_TX_MODE_LEFT, 0);
    
    
    return state;
    
}

u32 Reader_GetWight(GPB_INFO *gpbInfo)
{
    u32 tempValue = 0;

    u32 value = Sound_GetValue(g_sGpbInfo.wightTemp);
    //tempValue =  g_sGpbInfo.wightValue - value;
    
    tempValue = g_sGpbInfo.wightValue - value;
    /*
    if((g_sGpbInfo.wightTemp & 0x10000000) && g_sDeviceParamenter.reWorkMode == READER_MODE_TEST )           
    {
        tempValue =  g_sGpbInfo.wightValue + value;
    }
    else
    {
        if(g_sGpbInfo.wightValue - value > 0)
        {
          tempValue =  g_sGpbInfo.wightValue - value;        
        }
        else
        {
          tempValue =  value - g_sGpbInfo.wightValue;
        }
     
    }
*/


    return tempValue;
    

}

void Reader_ChgTag(u8 stat)                                                                                               //测试模式RFID状态区分              
{
    if(stat == RFID_TAG_IN)
    {
        char Buf[LCM_TXT_LEN_MAX] = "进场";     
        Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_TAG_STAT, sizeof(Buf), Buf, LCM_DISH_TX_MODE_LEFT, 0);
    }
    else if(stat == RFID_TAG_KEEP)
    {
        char Buf[LCM_TXT_LEN_MAX] = "存在";     
        Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_TAG_STAT, sizeof(Buf), Buf, LCM_DISH_TX_MODE_LEFT, 0);
    }
    else if(stat == RFID_TAG_OUT)
    {
        char Buf[LCM_TXT_LEN_MAX] = "离场";     
        Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_TAG_STAT, sizeof(Buf), Buf, LCM_DISH_TX_MODE_LEFT, 0);
    }
}


void Reader_ChgKey(u8 stat)                                                                                                    
{
 
}


void Reader_DisplayLable(DISH_INFO *pBuffer)                                                                                    //图标展示
{ 
    char tagInfoNull[LCM_TXT_TAG_LEN] = "";
    switch(pBuffer->tagNum)
    {
        case 0:
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_1, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_2, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_3, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_4, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_1);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_2);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_3);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_4);
        break;
        case 1:
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_1, LCM_TXT_TAG_LEN, pBuffer->tagInfo[0].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_2, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_3, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_4, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_1);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_2);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_3);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_4);
        break; 
        case 2:
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_1, LCM_TXT_TAG_LEN, pBuffer->tagInfo[0].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_2, LCM_TXT_TAG_LEN, pBuffer->tagInfo[1].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_3, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_4, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_1);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_2);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_3);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_4);
        
        break;
        case 3:
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_1, LCM_TXT_TAG_LEN, pBuffer->tagInfo[0].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_2, LCM_TXT_TAG_LEN, pBuffer->tagInfo[1].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_3, LCM_TXT_TAG_LEN, pBuffer->tagInfo[2].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_4, LCM_TXT_TAG_LEN, tagInfoNull, LCM_DISH_TX_MODE_MID, 0);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_1);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_2);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_3);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_TAG_4);
        break;
        case 4:
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_1, LCM_TXT_TAG_LEN, pBuffer->tagInfo[0].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_2, LCM_TXT_TAG_LEN, pBuffer->tagInfo[1].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_3, LCM_TXT_TAG_LEN, pBuffer->tagInfo[2].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_TAG_4, LCM_TXT_TAG_LEN, pBuffer->tagInfo[3].tagBuffer, LCM_DISH_TX_MODE_MID, 0);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_1);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_2);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_3);
            Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_TAG_4);
        break;
    
    
    }
}


u16 Reader_ProcessUartFrame(u8 *pFrame, u16 len)
{       
    u8 cmd = 0;
    u16 destAddr = 0;
    BOOL bRfOperation = FALSE;
    u16 paramsLen = 0;
    u8 bOk = 0;
    u32 rtc = 0;

    destAddr = *((u16 *)(pFrame + UART_FRAME_POS_DESTADDR));
    memcpy(&rtc, pFrame + READER_MEAL_RTC_LEN - 1, 4);
    if((destAddr != READER_FRAME_BROADCAST_ADDR) && (destAddr != g_sDeviceParamenter.addr))
    {
        return 0;
    }
    g_sDeviceRspFrame.destAddr = *((u16 *)(pFrame + UART_FRAME_POS_SRCADDR));
    g_sDeviceRspFrame.len = 0;
    cmd = *(pFrame + UART_FRAME_POS_CMD);
    if(cmd == UART_FRAME_RESPONSE_FLAG)
    {
        cmd = *(pFrame + UART_FRAME_POS_CMD + 1);
    }

    g_sDeviceRspFrame.cmd = cmd;
    g_sDeviceRspFrame.flag = UART_FRAME_FLAG_OK;
    g_sDeviceRspFrame.err = UART_FRAME_RSP_NOERR;
    
    if(pFrame[UART_FRAME_POS_LEN] == 0)
    {
        paramsLen = *((u16 *)(pFrame + UART_FRAME_POS_PAR));
    }
    else
    {
        paramsLen = len - UART_FRAME_MIN_LEN;
    }
    switch(cmd)
    {
        case READER_CMD_RESET:
            if(paramsLen == 0)
            {
                g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
            }
            break;
        case READER_CMD_GET_VERSION:
            if(paramsLen == 0)
            {
                g_sDeviceRspFrame.len = Device_ResponseFrame((u8 *)READER_VERSION, READER_VERSION_SIZE, &g_sDeviceRspFrame);
            }
            break;
        case READER_CMD_GET_CPUID:
            if(paramsLen == 0)
            {
                g_sDeviceRspFrame.len = Device_ResponseFrame((u8 *)STM32_CPUID_ADDR, STM32_CPUID_LEN, &g_sDeviceRspFrame);
            }
        case READER_CMD_SET_CFG:
            if(paramsLen == 7)
            {   
                if((*(pFrame+READER_SET_CFG + 2) == READER_UI_MODE_WHITE) || (*(pFrame+READER_SET_CFG + 2) == READER_UI_MODE_BLACK) && 
                    (*(pFrame+READER_SET_CFG + 3) == READER_TOTAL_MODE_AIR) || (*(pFrame+READER_SET_CFG + 3) == READER_TOTAL_MODE_AFTER))
                {
                  
                    g_sDeviceParamenter.rfu1 = *(pFrame+READER_SET_CFG + 1);
                    g_sDeviceParamenter.uiMode = *(pFrame+READER_SET_CFG + 2);    //暂时锁死
                    g_sDeviceParamenter.totalMode = *(pFrame+READER_SET_CFG + 3);
                    memcpy(&g_sDeviceParamenter.scoWt, pFrame + READER_SET_CFG + 4, 4);
                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                    Reader_WriteDeviceParamenter();
                    Reader_Font_ChgCorol(g_sDeviceParamenter.uiMode);

                }
                else
                {
                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_FAIL;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_FAIL;
                
                }
                    g_sDeviceRspFrame.len = Reader_Format_Cfg(0, &g_sDeviceRspFrame, &g_sDeviceParamenter);

            }
        case READER_CMD_GET_CFG:
            if(paramsLen == 0)
            {
                g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                g_sDeviceRspFrame.len = Reader_Format_Cfg(1, &g_sDeviceRspFrame, &g_sDeviceParamenter);
            }
            break;
        case READER_CMD_GET_HEART:
            if(paramsLen > 12)
            {
                if( *(pFrame + READER_MEAL_NAME_LEN) < NAME_NUM  && *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) <= 35)
                {
                    memset(&g_sDishTempInfo, 0, sizeof(DISH_INFO));
                    memcpy(&g_sDishTempInfo.dishName, pFrame + READER_MEAL_NAME_LEN, *(pFrame + READER_MEAL_NAME_LEN) + 1);
                    g_sDishTempInfo.tagNum = Reader_Lable(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1, &g_sDishTempInfo);
                    memcpy(&g_sDishTempInfo.unitPrice,pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN)+ 1 + *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) + 1, 4);
                    memcpy(&g_sDishTempInfo.spec,pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN)+ 1 + *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) + 1 + 4, 2);
                    g_sDishTempInfo.type = *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN)+ 1 + *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) + 1 + 4 + 2);
                    if((memcmp(&g_sRaderInfo.dishInfo.dishName, &g_sDishTempInfo.dishName, NAME_NUM)) || (memcmp(&g_sRaderInfo.dishInfo.tagInfo, &g_sDishTempInfo.tagInfo, sizeof(DISH_TAG_INFO))) || (memcmp(&g_sRaderInfo.dishInfo.unitPrice, &g_sDishTempInfo.unitPrice, 4)) ||
                       (memcmp(&g_sRaderInfo.dishInfo.spec, &g_sDishTempInfo.spec, 2)) || (memcmp(&g_sRaderInfo.dishInfo.type, &g_sDishTempInfo.type, 1)))
                    {
                        memcpy(&g_sRaderInfo.dishInfo.dishName, &g_sDishTempInfo.dishName, NAME_NUM);
                        //memcpy(&g_sRaderInfo.dishInfo.tagInfo,  &g_sDishTempInfo.tagInfo, sizeof(DISH_TAG_INFO) * 4);
                        memcpy(&g_sRaderInfo.dishInfo.unitPrice, &g_sDishTempInfo.unitPrice, 4);
                        memcpy(&g_sRaderInfo.dishInfo.spec,  &g_sDishTempInfo.spec, 2);
                        memcpy(&g_sRaderInfo.dishInfo.type, &g_sDishTempInfo.type, 1);
                        g_sRaderInfo.dishInfo.tagNum = Reader_Lable(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1, &g_sRaderInfo.dishInfo);
                        g_sRaderInfo.dishInfo.state = READER_DISH_INFO_CHG;
                    }

                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                    g_nTempLink = READER_LINK_OK;
                    g_nTickLink = g_nSysTick;
                    if((Lcm_ChkPage(LCM_FLAG_PAGE_NULL_CHG) || Lcm_ChkPage(LCM_FLAG_PAGE_NULL_CHG + READER_UI_MODE_BLACK)) && ! Lcm_ChkPage(LCM_FLAG_PAGE_TEST))
                    {
                        Lcm_SetPage(LCM_FLAG_PAGE_MAIN_CHG); 
                    }
                       
                }
                else
                {
                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
               }
               
               g_sDeviceRspFrame.len = Reader_Format_Heart(&g_sDeviceRspFrame, rtc, Sound_GetValue(g_sGpbInfo.wightTemp));   //重量数据代填
            }
            break;
        case READER_CMD_GET_PERSON_INFO:
            if(paramsLen == 21)
            {
                bOk = *(pFrame + READER_PERSON_MONEY_LEN + 1 + 1);
                if(Reader_ChkReUid(pFrame + READER_PERSON_UID_LEN, g_sReaderRfidTempInfo.uid) && bOk == READER_RES_OK)
                {
                    //memcpy(&g_sRaderInfo.personInfo, 0 , sizeof(PERSON_INFO));
                    memcpy(&g_sRaderInfo.personInfo.money, pFrame + READER_PERSON_UID_LEN + 12, 4);
                    g_sRaderInfo.personInfo.state = READER_RESFRAME_PERSON_OK;
                    g_sRaderInfo.total = *(pFrame + READER_PERSON_MONEY_LEN);
                    g_sRaderInfo.personInfo.allergy = *(pFrame + READER_PERSON_MONEY_LEN );
                    g_sRaderInfo.personInfo.bind = *(pFrame + READER_PERSON_MONEY_LEN + 1);
                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                }
                else
                {
                    g_sRaderInfo.personInfo.state = READER_RESFRAME_PERSON_FAIL;
                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                }
                g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
            }
            break;
        case READER_CMD_GET_MEAL_INFO:
            if(paramsLen > 1)
            {
                bOk = *(pFrame + READER_PERSON_UID_LEN + 12);
                if(Reader_ChkReUid(pFrame + READER_PERSON_UID_LEN, g_sReaderRfidTempInfo.uid) && bOk == READER_RES_OK)
                {
                  //上传成功
                    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                    g_sDeviceRspFrame.state = READER_DATA_UP_OK;
                    
                  
                }

                g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
            }
      break;
        case READER_CMD_CHG_IP:
            if(paramsLen > 1)
            {
                g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
                g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
                g_sDeviceRspFrame.state = READER_DATA_UP_OK;

                g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
            }
      break;
    
    }

     if(g_sDeviceRspFrame.len == 0 && bRfOperation == FALSE)
    {
        g_sDeviceRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
        g_sDeviceRspFrame.err = READER_OPTAG_RESPONSE_PARERR;
        g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
    }
    
    return g_sDeviceRspFrame.len;//
}


u8 Device_ResponseFrame(u8 *pParam, u8 len, READER_RSPFRAME *pOpResult)
{
    u16 pos = 0;
    u16 crc = 0;

    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = UART_FRAME_RESPONSE_FLAG;
    pOpResult->buffer[pos++] = pOpResult->cmd;               // cmd

    if(len > UART_FRAME_PARAM_MAX_LEN)
    {
        pOpResult->buffer[pos++] = (len >> 0) & 0xFF;
        pOpResult->buffer[pos++] = (len >> 8) & 0xFF;

        memcpy(pOpResult->buffer + pos, pParam, len);
        pos += len;
    }
    else
    {
        if(pOpResult->flag == READER_RSPFRAME_FLAG_OK)
        {
            memcpy(pOpResult->buffer + pos, pParam, len);
        }
        else
        {
            memset(pOpResult->buffer + pos, 0, len);
        }
        pos += len;
        pOpResult->buffer[pos++] = pOpResult->flag;
        pOpResult->buffer[pos++] = pOpResult->err;
        pOpResult->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    }

    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;

    pOpResult->len = pos;

    return pos;
}


u8 Reader_Format_GetPerson(u8 *puid,  READER_RSPFRAME *pOpResult, u32 rtc)
{
    u16 pos = 0;
    u16 crc = 0;
    
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = READER_CMD_GET_PERSON_INFO;
    pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;
    memcpy(pOpResult->buffer + pos, puid, ISO15693_SIZE_UID);
    pos += ISO15693_SIZE_UID;
    pOpResult->buffer[pos++] = (rtc >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 24) & 0xFF; 
    
    pOpResult->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;

    pOpResult->len = pos;
    return pos;


}


u8 Reader_Format_Meal(u8 type,u8 *puid, READER_RSPFRAME *pOpResult, u32 rtc, u32 witgh)
{
    u16 pos = 0;
    u16 crc = 0;
    
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = READER_CMD_GET_MEAL_INFO;
    pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;
    memcpy(pOpResult->buffer + pos, puid, ISO15693_SIZE_UID);
    pos += ISO15693_SIZE_UID;
    pOpResult->buffer[pos++] = (rtc >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 24) & 0xFF;

    pOpResult->buffer[pos++] = (witgh >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 24) & 0xFF;

    pOpResult->buffer[pos++] = type & 0xFF;

    pOpResult->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;

    pOpResult->len = pos;
    return pos;


}


u8 Reader_Format_Heart( READER_RSPFRAME *pOpResult, u32 rtc,u32 witgh)
{
    u16 pos = 0;
    u16 crc = 0;
    
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = pOpResult->cmd;;
    pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;

    pOpResult->buffer[pos++] = (rtc >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 24) & 0xFF;

    
    pOpResult->buffer[pos++] = (witgh >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 24) & 0xFF;

    pOpResult->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;


    pOpResult->len = pos;
    return pos;
}

u8 Reader_Format_Cfg(u8 mode, READER_RSPFRAME *pOpResult, READER_DEVICE_PARAMETER *pParameter)
{
    u16 pos = 0;
    u16 crc = 0;
    
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = UART_FRAME_RESPONSE_FLAG;
    pOpResult->buffer[pos++] = pOpResult->cmd;;
    pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;
    if(mode)
    {
        pOpResult->buffer[pos++] = pParameter->rfu1;
        pOpResult->buffer[pos++] = pParameter->uiMode;
        pOpResult->buffer[pos++] = pParameter->totalMode;
        
        pOpResult->buffer[pos++] = (pParameter->scoWt >> 0) & 0xFF;
        pOpResult->buffer[pos++] = (pParameter->scoWt >> 8) & 0xFF;
        pOpResult->buffer[pos++] = (pParameter->scoWt >> 16) & 0xFF;
        pOpResult->buffer[pos++] = (pParameter->scoWt >> 24) & 0xFF;

        memcpy(pOpResult->buffer + pos, g_aReaderVersion, READER_VERSION_LEN);
        pos += READER_VERSION_LEN; 
        pOpResult->buffer[pos++] = 0x00;                //配置标志，无用，暂留
        pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;
    }
    else
    {   
        memcpy(pOpResult->buffer + pos, g_aReaderVersion, READER_VERSION_LEN);
        pos += READER_VERSION_LEN;
        pOpResult->buffer[pos++] = pOpResult->flag;
        pOpResult->buffer[pos++] = pOpResult->err;//错误类别，暂未设立
    }


    pOpResult->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    crc = a_GetCrc(pOpResult->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;

    return pos;
}

u8 Reader_Lable(u8 *pBuffer, DISH_INFO *pDishInfo)                          //标签处理
{
    u8 mark = 0x7C, i = 0, len = 0, num = 0, j = 0;     //'\'分离
    u8 temp[4] = {0};
    len = pBuffer[0];
      

    for(i = 0; i <= len ; i++)
    {
        if(pBuffer[i] == mark)
        {
            num ++;
            temp[j] = i ;
            j++;
        }
    }

    if(num == 0)
    {
        if(pBuffer[1] == 0x00 && pBuffer[2] == 0x00 && pBuffer[3] == 0x00 && pBuffer[4] == 0x00 )
        {
            num = 5;
        }
        else
        {
            memcpy(pDishInfo->tagInfo + 0, pBuffer + 1,  len - temp[0]);
        }

    }
    else if(num == 1)
    {
        memcpy(pDishInfo->tagInfo + 0, pBuffer + 1,  temp[0] - 1);
        memcpy(pDishInfo->tagInfo + 1, pBuffer + temp[0] + 1, len - temp[0] );
    }
    else if(num == 2)
    {
        memcpy(pDishInfo->tagInfo + 0, pBuffer + 1,  temp[0] - 1);
        memcpy(pDishInfo->tagInfo + 1, pBuffer + temp[0] + 1, temp[1] - temp[0] - 1);
        memcpy(pDishInfo->tagInfo + 2, pBuffer + temp[1] + 1, len - temp[1]  );
        
    }
    else if(num == 3)
    {
        memcpy(pDishInfo->tagInfo + 0, pBuffer + 1,  temp[0] - 1);
        memcpy(pDishInfo->tagInfo + 1, pBuffer + temp[0] + 1, temp[1] - temp[0] - 1);
        memcpy(pDishInfo->tagInfo + 2, pBuffer + temp[1] + 1, temp[2] - temp[1]  - 1);
        memcpy(pDishInfo->tagInfo + 3, pBuffer + temp[2] + 1, len - temp[2]);
        
    }

    return ++num;
}

void Reader_ChgPage(LCM_INFO *pLcmInfo)
{
    if(Lcm_ChkChg(pLcmInfo))
    {
        Lcm_ChgPage(pLcmInfo);
        if(pLcmInfo->flag == LCM_FLAG_PAGE_TEST &&  g_sDeviceParamenter.uiMode == READER_UI_MODE_BLACK)
        {
            Lcm_SelectPage(LCM_FLAG_PAGE_TEST);
        }
        else
        {
            if(pLcmInfo->page == LCM_FLAG_PAGE_NULL_CHG + READER_UI_MODE_BLACK && g_sDeviceParamenter.uiMode == READER_UI_MODE_BLACK)
            {
                 Lcm_SelectPage(pLcmInfo->page);
            }
            else
            {
                Lcm_SelectPage(pLcmInfo->page + g_sDeviceParamenter.uiMode);
            }
           
        }
        
    }

}

void Reader_DisplayDish(DISH_INFO *pDishInfo, GPB_INFO *gpbInfo, LCM_INFO *pLcmInfo)
{
    char Buf[LCM_TXT_LEN_MAX] = ""; 
    char nullBuf[LCM_TXT_LEN_MAX] = ""; 
    u32 tempValue = 0;
    
    
    tempValue = Reader_GetWight(&g_sGpbInfo);
    switch(pLcmInfo->page)
    {
        case LCM_PAGE_MAIN_WHITE:
          
            if(g_sRaderInfo.dishInfo.state == READER_DISH_INFO_CHG)
            {
                Reader_DisplayLable(&g_sRaderInfo.dishInfo);
                Lcm_DishWriteTxt(LCM_TXT_ADDR_NAME, NAME_NUM / 2, (char *)pDishInfo->dishName + 1, LCM_DISH_TX_MODE_LEFT, 0);
                if(* pDishInfo->dishName > NAME_NUM / 2)
                {
                    Lcm_DishWriteTxt(LCM_TXT_ADDR_NAME_2, NAME_NUM / 2, (char *)pDishInfo->dishName + 1 + NAME_NUM / 2, LCM_DISH_TX_MODE_LEFT, 0);
                }
                else
                {
                    Lcm_DishWriteTxt(LCM_TXT_ADDR_NAME_2, NAME_NUM / 2, nullBuf, LCM_DISH_TX_MODE_LEFT, 0);
                }
                Reader_ChkMoney(pDishInfo->unitPrice, Buf);
                Lcm_DishWriteUniCodeTxt(LCM_TXT_ADDR_PEICE, 5, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                sprintf(Buf, "元/%d克",pDishInfo->spec);
                Lcm_DishWriteTxt(LCM_TXT_ADDR_SPEC, 10, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                g_sRaderInfo.dishInfo.state = READER_DISH_INFO_NO_CHG;
            }
            Lcm_DishWriteTxt(LCM_TXT_ADDR_LINK_STATE, 8, g_nBufTxt1, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_INFO, LCM_INFO_TXT_SIZE, g_nBufTxt, LCM_DISH_TX_MODE_MID, 0);
          break;        
        case LCM_PAGE_INFO_WHITE:

            if( g_sRaderInfo.dishInfo.state == READER_DISH_INFO_CHG)
            {
                Reader_DisplayLable(&g_sRaderInfo.dishInfo);
                Lcm_DishWriteTxt(LCM_TXT_ADDR_NAME, NAME_NUM / 2, (char *)pDishInfo->dishName + 1, LCM_DISH_TX_MODE_LEFT, 0);
                if(* pDishInfo->dishName > NAME_NUM / 2)
                {
                    Lcm_DishWriteTxt(LCM_TXT_ADDR_NAME_2, NAME_NUM / 2, (char *)pDishInfo->dishName + 1 + NAME_NUM / 2, LCM_DISH_TX_MODE_LEFT, 0);
                }
                else
                {
                    Lcm_DishWriteTxt(LCM_TXT_ADDR_NAME_2, NAME_NUM / 2, nullBuf, LCM_DISH_TX_MODE_LEFT, 0);
                }
                Reader_ChkMoney(pDishInfo->unitPrice, Buf);
                Lcm_DishWriteUniCodeTxt(LCM_TXT_ADDR_PEICE, 5, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                sprintf(Buf, "元/%d克",pDishInfo->spec);
                Lcm_DishWriteTxt(LCM_TXT_ADDR_SPEC, 10, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                g_sRaderInfo.dishInfo.state = READER_DISH_INFO_NO_CHG;
            }
        
            Lcm_DishWriteTxt(LCM_TXT_ADDR_ARRERGY, LCM_TIP_TXT_SIZE, g_nBufTxt, LCM_DISH_TX_MODE_MID, 0);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_LINK_STATE, 8, g_nBufTxt1, LCM_DISH_TX_MODE_MID, 0);

            if((g_sGpbInfo.wightValue >= Sound_GetValue(g_sGpbInfo.wightTemp)) && (Reader_GetWight(&g_sGpbInfo) >=  g_sDeviceParamenter.scoWt) && !g_nReaderGpbState)  
            {
                pDishInfo->peice = tempValue * (pDishInfo->unitPrice) / (pDishInfo->spec);
                g_nReaderGpbState = TRUE;
            }
            else if(g_nReaderGpbState)
            {  
                tempValue = Reader_GetWight(&g_sGpbInfo);
                pDishInfo->peice = tempValue * (pDishInfo->unitPrice) / (pDishInfo->spec);
            }
            else 
            {        
                tempValue = 0;
                pDishInfo->peice = 0;
            }
                        
            if(tempValue & 0x80000000)
            {        
                tempValue = 0;
                pDishInfo->peice = 0;
            }
                       
            if(tempValue)
            {
                        
                        
            }
            sprintf(Buf, "%d", tempValue);  
            Lcm_DishWriteUniCodeTxt(LCM_TXT_ADDR_WIGHT, 5, Buf, LCM_DISH_TX_MODE_MID, 0);
            Reader_ChkMoney(pDishInfo->peice, Buf);
            Lcm_DishWriteUniCodeTxt(LCM_TXT_ADDR_MONEY, 8, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                        
            sprintf(Buf, "%d克",g_sDeviceParamenter.scoWt);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_SCOOP_WTGHT, 8, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                        
            Reader_ChkMoney(g_sDeviceParamenter.scoWt * (pDishInfo->unitPrice) / (pDishInfo->spec), Buf);
            Lcm_DishWriteTxt(LCM_TXT_ADDR_SCOOP_MONEY, 7, Buf, LCM_DISH_TX_MODE_LEFT, 0);
                        
            Reader_ChkMoney(g_sRaderInfo.personInfo.money + pDishInfo->peice , Buf);
            Lcm_DishWriteUniCodeTxt(LCM_TXT_ADDR_TOTAL, 8, Buf, LCM_DISH_TX_MODE_LEFT, 0);
          break;
          case LCM_PAGE_TEST:
                    
            
            break;
         case LCM_FLAG_PAGE_LINK_CHG:
                    
            //Lcm_SetPage(LCM_FLAG_PAGE_LINK_CHG);
            break;
             
     
    }

}


BOOL Reader_ChkReUid(u8 *pRUid, u8 *pTUid)
{
    BOOL b = TRUE;
    u8 i = 0;
    for(i = 0; i < 8; i++)
    {
        if(pRUid[i] != pTUid[i])
        {
            b = FALSE;
            break;
        }
    
    }

    return b;
}

void Reader_ChkMoney(u32 value, char *buf)
{
    if(value < 1000)
    {
         sprintf(buf,"%d.%d%d", value / 100, (value % 100) / 10,  (value % 100) % 10);    
    }
    else if(1000 <= value && value < 10000)
    {
         sprintf(buf,"%d%d.%d%d", value / 1000, (value % 1000) / 100, (value % 100) / 10, (value % 100) % 10);    
    }
    else if(10000 <= value && value < 100000)
    {
        sprintf(buf,"%d%d%d.%d%d",value / 10000, (value % 10000) / 1000, (value % 1000) / 100, (value % 100) / 10, (value % 100) % 10); 
    }
    else if(100000 <= value && value < 1000000)
    {
        sprintf(buf,"%d%d%d%d.%d%d",value / 100000, (value % 100000) / 10000, (value % 10000) / 1000, (value % 1000) / 100, (value % 100) / 10, (value % 100) % 10); 
    }
}

void Reader_Font_ChgCorol(u8 background)
{
    if(background == READER_UI_MODE_BLACK)
    {
        Lcm_DishWriteColor(LCM_TXT_ADDR_LINK_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_NAME_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_INFO_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_SPEC_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_MAIN_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_NO_MEAL_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_NAME_2_COLOR, LCM_FONT_COROL_WHITE);
        
        //Lcm_DishWriteColor(LCM_TXT_ADDR_LINK_TXT_COLOR, LCM_FONT_COROL_WHITE);
       // Lcm_DishWriteColor(LCM_TXT_ADDR_SCOOP_MONEY_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_SCOOP_WITGH_COLOR, LCM_FONT_COROL_WHITE);
    }
    else
    {
        Lcm_DishWriteColor(LCM_TXT_ADDR_LINK_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_NAME_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_INFO_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_SPEC_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_MAIN_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_NO_MEAL_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_LINK_TXT_COLOR, LCM_FONT_COROL_BLACK);
        Lcm_DishWriteColor(LCM_TXT_ADDR_NAME_2_COLOR, LCM_FONT_COROL_BLACK);
        //Lcm_DishWriteColor(LCM_TXT_ADDR_SCOOP_MONEY_COLOR, LCM_FONT_COROL_WHITE);
        //Lcm_DishWriteColor(LCM_TXT_ADDR_SCOOP_WITGH_COLOR, LCM_FONT_COROL_WHITE);
        Lcm_DishWriteColor(LCM_TXT_ADDR_LINK_INFO_COLOR, LCM_FONT_COROL_BLACK);
    }
    Lcm_SelectPage(g_sLcmInfo.page + g_sDeviceParamenter.uiMode);
}

void Reader_OffLineClear()
{
    BOOL b = FALSE; 
    
    g_sFlashData.id = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);
       
    if(b)
    {
        g_sReaderOffLineInfo.readerOffLineData[g_sFlashData.id >> 3] &= ~(0x80 >> (g_sFlashData.id & 0x07));
    }
    g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
        

}

