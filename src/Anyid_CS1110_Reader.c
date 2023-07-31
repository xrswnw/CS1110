#include "AnyID_CS1110_Reader.h"

const u8 READER_VERSION[READER_VERSION_SIZE]@0x08005000 = " CS1110 23073102 G230200";

u32 g_nReaderState = READER_STAT_IDLE;

DISH_INFO g_sDishTempInfo = {0};
READER_DEVICE_PARAMETER g_sDeviceParamenter = {0};
READER_INFO g_sRaderInfo ={0};
READER_RFID_INFO g_sReaderRfidTempInfo= {0};
READER_RSPFRAME g_sDeviceRspFrame = {0};
READER_OFFLINE_INFO g_sReaderOffLineInfo = {0};
READER_LED_INFO g_sReaderLedInfo = {0};
READER_RSPFRAME g_sDeviceMealRspFrame = {0};


BOOL g_nReaderGpbState = FALSE;
u8 g_aReaderISO15693Uid[READER_OP_UID_MAX_NUM * ISO15693_SIZE_UID] = {0};
u8 g_nTempLink = 0;
u32 g_nTickLink = 0;
u32 g_nRtcTime = 0;
u32 g_nOffLineLimitTime = 0;

int g_nReaderWightValue = 0;
char g_nBufTxt[LCM_TXT_LEN_MAX] = "未绑定托盘"; 
char g_nBufTxt1[LCM_TXT_LEN_MAX] = ""; 


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
	g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
	g_sReaderOffLineInfo.boundaryNum = g_sReaderOffLineInfo.num;
    g_sDeviceParamenter.reWorkMode = READER_MODE_NORMAL;
    Reader_Font_ChgCorol(g_sDeviceParamenter.uiMode);
}



BOOL Reader_ReadDeviceParamenter(void)									     //OK
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
        g_sDeviceParamenter.ip[2] = 2;
        g_sDeviceParamenter.ip[3] = 10;
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


BOOL Reader_ReadOffLineDataNum(void)									     //OK
{
    BOOL b = FALSE ;
    
    b = FRam_ReadBuffer(FRAM_OFF_LINE_NUM_ADDR, sizeof(READER_OFFLINE_INFO), (u8 *)(&g_sReaderOffLineInfo));
 
    return b;
    
}

BOOL Reader_ReadOffLineDatas(u16 addr, u16 size, u8 *pBuffer)									  //OK
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


u8 Reader_RfidGetValue(u8 *pBuffer, READER_RFID_INFO *pRfidInfo)
{
	
	
    u8 state = 0;
    u8 i = 0;
    u8 tempUid[READER_UID_MAX_NUM * ISO15693_SIZE_UID] = {0};
    u8 num = 0;
    static BOOL tempState = FALSE;
    static u8 linkTick = 0;
    num = *(pBuffer + READER_RFID_UID_POST);
    pRfidInfo->allTick ++;
    pRfidInfo->tick++;
    pRfidInfo->num = num;
    if(num)
    {
        if(!tempState)
        {
			linkTick = RFID_ONE_TAG_TIME;
        }
        else
        {
			linkTick = RFID_ONE_MORE_TIME;
        }
        if(num == 1)
        {
			tempState = FALSE;
			pRfidInfo->okTick++;
			pRfidInfo->succesTick ++;  
			pRfidInfo->errTick = 0;
			pRfidInfo->clashTicks = 0;
			g_sRfidInfo.flag = RFID_TAG_IN;
			memcpy(tempUid, pBuffer + READER_RFID_UID_POST + 1 , ISO15693_SIZE_UID);
			if(pRfidInfo->okTick == linkTick)
			{
			    pRfidInfo->state = RFID_TAG_IN;
			    memcpy(pRfidInfo->uid, tempUid, ISO15693_SIZE_UID);
			}
			else if((pRfidInfo->okTick >= linkTick))						  //标签重叠，由多便签变化为单标签，动态增加，后续归为
			{
			     pRfidInfo->state = RFID_TAG_KEEP;
			    for(i = 0 ; i < ISO15693_SIZE_UID; i++)
			    {
			        if(pRfidInfo->uid[i] != tempUid[i])
			        {
						pRfidInfo->clashTick ++;
						break;
			        }
					else
					{
						pRfidInfo->clashTick = 0;
					}
			    }
				if(pRfidInfo->clashTick > RFID_ONE_TAG_TIME)
				{
					pRfidInfo->okTick = 0;
			        pRfidInfo->errTick += READER_RFID_MOVE_TIME;       //数据传输问题
			        pRfidInfo->state = RFID_TAG_OUT;
				}
				
			}
			
			if(g_nReaderWightValue != READER_WIGHT_MAX_VALUE)
			{
				g_nReaderWightValue = READER_WIGHT_MAX_VALUE;
			}
        }
        else
        {
			//多餐盘
			tempState = TRUE;
			if(pRfidInfo->clashTicks >= RFID_ONE_TAG_TIME)
			{
				pRfidInfo->clashTicks = 0;
				pRfidInfo->errTick += READER_RFID_MOVE_TIME;
				pRfidInfo->state = RFID_TAG_OUT;
				pRfidInfo->clash = RFID_TAG_FAIL;
			}
			else
			{
				pRfidInfo->clashTicks++;
			}
        
        
			
        }

    }
    else
    {
        pRfidInfo->errTick ++;
		if(pRfidInfo->errTick > RFID_ONE_TAG_TIME)									  //冲突问题，可能导致标签在位也读取失败
        {  
			g_sRfidInfo.flag = RFID_TAG_OUT;
        }
        if(pRfidInfo->errTick >= READER_RFID_MOVE_TIME)									  //餐盘离开后1.5s上报数据
        {  
			pRfidInfo->okTick = 0;
			pRfidInfo->state = RFID_TAG_OUT;
        }
    }
/*
	u8 num = 0;
	u8 state = 0;
	
    num = *(pBuffer + READER_RFID_UID_POST);	
	pRfidInfo->num = num;
	pRfidInfo->allTick++;
	if(num == 1)
	{
		pRfidInfo->succesTick++;
	}
*/

    return state;
}

u8 Reader_DisplayTest(DISH_INFO *pDishInfo, WIGHT_INFO *witghInfo)															      
{
	char Buf[LCM_TXT_LEN_MAX] = ""; 
    u32 percentage = 0;
    u8 state = 0;
    
	percentage = g_sReaderRfidTempInfo.succesTick * 10000 / g_sReaderRfidTempInfo.allTick;
	
	
    if(g_sGpbInfo.flag  == GPB_FLAG_NORMAL)
    {
		sprintf(Buf, "%dg", g_sGpbInfo.wightTemp); 
    }
    else
    {
        sprintf(Buf, "失联");
    }
    
    if(percentage == 0xFFFF)
    {
        percentage = LCM_TXT_SUCCES_FULL;
    }
     
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_1, LCM_TXT_TEST_LEN, Buf, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
    sprintf(Buf, "%d个", g_sReaderRfidTempInfo.num);   
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_BUTTON_2, LCM_TXT_TEST_BUTTON_LEN, Buf, LCM_DISH_TX_MODE_MID, LCM_TXT_OFFSET_NULL);
    sprintf(Buf, "%d", g_sReaderRfidTempInfo.allTick );    
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_2, LCM_TXT_TEST_LEN, Buf, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
	sprintf(Buf, "%d.%d", percentage / 100, percentage % 100);
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_3, LCM_TXT_TEST_LEN, Buf, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
    
    return state;
    
}



void Reader_ChgStat(u8 lineState)																								    
{
    static u8 state = 0xFF;
    char Buf[LCM_TXT_LEN_MAX];
    if(state != lineState)
    {
        state = lineState;
        if(state == READER_LINK_FAIL)
        {
			memcpy(Buf, "离线", LCM_TXT_LEN_MAX);   
			Lcm_DishWriteColor(LCE_TXT_ADDR_LINE_STAT, LCM_FONT_COROL_RED);
			Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_TAG_STAT, sizeof(Buf), Buf, LCM_DISH_TX_MODE_LEFT, 0);
        }
        else
        {
			 memcpy(Buf, "在线", LCM_TXT_LEN_MAX); 
			 Lcm_DishWriteColor(LCE_TXT_ADDR_LINE_STAT, LCM_FONT_COROL_GREEN);
			Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_TAG_STAT, sizeof(Buf), Buf, LCM_DISH_TX_MODE_LEFT, 0);
        }
        
    }
    else
    {
        state = lineState;
    }

}


void Reader_DisplayLable(DISH_INFO *pBuffer)																					//图标展示
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
	u16 deviceAddr = 0;
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
	if(cmd != READER_CMD_CHG_IP)
	{
		g_nTempLink = READER_LINK_OK;
		g_nTickLink = g_nSysTick;
	}
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
			    g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
			    g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
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
			    u32 rtcTime = 0;
			    if(!a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
			    {
			        if( 0 < *(pFrame + READER_MEAL_NAME_LEN) < 64  && *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) <= 35)
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
						    memcpy(&g_sRaderInfo.dishInfo.unitPrice, &g_sDishTempInfo.unitPrice, 4);
						    memcpy(&g_sRaderInfo.dishInfo.spec,  &g_sDishTempInfo.spec, 2);
						    memcpy(&g_sRaderInfo.dishInfo.type, &g_sDishTempInfo.type, 1);
						    g_sRaderInfo.dishInfo.tagNum = Reader_Lable(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1, &g_sRaderInfo.dishInfo);
						    g_sRaderInfo.dishInfo.state = READER_DISH_INFO_CHG;
						}

						memcpy(&rtcTime, pFrame + READER_MEAL_RTC_POS, 4);
						g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
						g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
						
						RTC_SetTime(rtcTime);
						if((Lcm_ChkPage(LCM_FLAG_PAGE_NULL_CHG) || Lcm_ChkPage(LCM_FLAG_PAGE_NULL_CHG + READER_UI_MODE_BLACK)) && ! Lcm_ChkPage(LCM_FLAG_PAGE_TEST))
						{
						    Lcm_SetPage(LCM_FLAG_PAGE_MAIN_CHG); 
						}
			        }
					else
					{
						memset(&g_sRaderInfo.dishInfo, 0, sizeof(DISH_INFO));
						Lcm_SetPage(LCM_FLAG_PAGE_NULL_CHG); 
					}
			    }
			    else
			    {
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_FAIL;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_FAIL;
			   }
			   g_sDeviceRspFrame.len = Reader_Format_Heart(&g_sDeviceRspFrame, rtcTime, g_sGpbInfo.wightTemp);   //重量数据代填
			}
			break;
        case READER_CMD_GET_PERSON_INFO:
			if(paramsLen == 21)
			{
			    bOk = *(pFrame + READER_PERSON_MONEY_LEN + 1 + 1);
			    if(bOk == READER_RES_OK && !memcmp(pFrame + READER_PERSON_RTC_LEN, &(g_sDeviceRspFrame.rtcTime), 4)) 
			    {
			        memcpy(&g_sRaderInfo.personInfo.money, pFrame + READER_PERSON_UID_LEN + 12, 4);
			        g_sRaderInfo.personInfo.state = READER_RESFRAME_PERSON_OK;
			        g_sRaderInfo.total = *(pFrame + READER_PERSON_MONEY_LEN);
			        g_sRaderInfo.personInfo.allergy = *(pFrame + READER_PERSON_MONEY_LEN );
			        g_sRaderInfo.personInfo.bind = *(pFrame + READER_PERSON_MONEY_LEN + 1);
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
			        g_sDeviceRspFrame.succFlag = READER_RESFRAME_PERSON_OK;
					g_sDeviceRspFrame.status = 0;
			    }
			    else
			    {
			        g_sRaderInfo.personInfo.state = READER_RESFRAME_PERSON_FAIL;
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_FAIL;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_FAIL;
			    }
			    g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
			}
			break;
        case READER_CMD_GET_MEAL_INFO:
			if(paramsLen > 1)
			{
			    bOk = *(pFrame + READER_PERSON_UID_LEN + 12);
			    u32 maskRtc = 0x00000000;
			    u64 maskUid = 0xFFFFFFFFFFFFFFFF;
			     if(bOk == READER_RES_OK  &&  !memcmp(pFrame + READER_PERSON_RTC_LEN, g_sDeviceMealRspFrame.buffer +  READER_PERSON_RTC_LEN -1, 4))
			    {
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
			        g_sDeviceMealRspFrame.state = READER_DATA_UP_OK;
					a_ClearStateBit(g_nReaderState, READER_STAT_UART_WAIT);
			    }
			    else if(bOk == READER_RES_OK  &&  !memcmp(pFrame + READER_PERSON_RTC_LEN, &(g_sDeviceRspFrame.rtcTime), 4))
			    {
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
			        g_sDeviceMealRspFrame.state = READER_DATA_UP_OK;
					a_ClearStateBit(g_nReaderState, READER_STAT_UART_WAIT);
			    }
			    else if(!memcmp(pFrame + READER_PERSON_UID_LEN, &maskUid, 8) && !memcmp(pFrame + READER_PERSON_RTC_LEN, &maskRtc, 4) && bOk == READER_RES_OK)
			    {
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_OK;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_OK;
			        g_sDeviceMealRspFrame.state = READER_DATA_UP_OK;
					a_ClearStateBit(g_nReaderState, READER_STAT_UART_WAIT);
			    }
			    else
			    {
			        g_sRaderInfo.personInfo.state = READER_RESFRAME_PERSON_FAIL;
			        g_sDeviceRspFrame.flag = READER_FRAME_FLAG_FAIL;
			        g_sDeviceRspFrame.err = READER_FRAME_INFO_FAIL;
			    }
			    g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
			}
      break;
        case READER_CMD_CHG_IP:
			if(paramsLen == READER_SFG_IP_LEN)
			{
			    if(g_sDeviceParamenter.ip[0] != *(pFrame + 9) || g_sDeviceParamenter.ip[1] != *(pFrame + 10) || 
			       g_sDeviceParamenter.ip[2] != *(pFrame + 11) || g_sDeviceParamenter.ip[3] != *(pFrame + 12))
			    {
			        g_sDeviceParamenter.ip[0] = *(pFrame + 9);
			        g_sDeviceParamenter.ip[1] = *(pFrame + 10);
			        g_sDeviceParamenter.ip[2] = *(pFrame + 11);
			        g_sDeviceParamenter.ip[3] = *(pFrame + 12);
			        Reader_WriteDeviceParamenter();
			        Reader_DisplayIp(&g_sDeviceParamenter);
			    }
			}
      break;
    case READER_CMD_DRU_CMD:
        if(paramsLen)
        {
			deviceAddr = *(pFrame + UART_FRAME_POS_PAR);
			if(deviceAddr == DEVICE_ADDR_READER)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_RFID;
			}
			else if(deviceAddr == DEVICE_ADDR_WIGHT)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_WIGHT;
			}
			else if(deviceAddr == DEVICE_ADDR_LCM)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_LCM;
			}
			else if(deviceAddr == DEVICE_ADDR_NULL)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_RESET;
			}
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
    pOpResult->status = READER_STATUS_ENTOURAGR;
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pOpResult->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pOpResult->buffer[pos++] = 0x00;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = UART_FRAME_RESPONSE_FLAG;
    pOpResult->buffer[pos++] = pOpResult->cmd;			   // cmd

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
    pOpResult->status = READER_STATUS_MASTER;
    pOpResult->rtcTime = rtc;
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
    
    pOpResult->rtcTime = rtc;
      
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
    
     pOpResult->rtcTime = rtc;   //存储上报RTC
    return pos;


}


u8 Reader_Format_Heart( READER_RSPFRAME *pOpResult, u32 rtc,u32 witgh)
{
    u16 pos = 0;
    u16 crc = 0;
    pOpResult->status = READER_STATUS_ENTOURAGR;
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

    pOpResult->buffer[pos++] = (rtc >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (rtc >> 24) & 0xFF;

    
    pOpResult->buffer[pos++] = (witgh >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 16) & 0xFF;
    pOpResult->buffer[pos++] = (witgh >> 24) & 0xFF;
    
    pOpResult->buffer[pos++] = pOpResult->flag;
    pOpResult->buffer[pos++] = pOpResult->err;//错误类别，暂未设立

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
    pOpResult->status = READER_STATUS_ENTOURAGR;
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

        memcpy(pOpResult->buffer + pos, (u8 *)&READER_VERSION + 8, READER_VERSION_LEN);
        pos += READER_VERSION_LEN; 
        pOpResult->buffer[pos++] = 0x00;			    //配置标志，无用，暂留
        pOpResult->buffer[pos++] = UART_FRAME_PARAM_RFU;
    }
    else
    {   
        memcpy(pOpResult->buffer + pos, (u8 *)&READER_VERSION + 8, READER_VERSION_LEN);
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

u8 Reader_Lable(u8 *pBuffer, DISH_INFO *pDishInfo)						  //标签处理
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
    if(len)
    {
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
    }
    else
    {
        return 0;
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
    
    tempValue =  Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue);
	
	
	//Uart_WriteBuffer((u8 *)&(g_sGpbInfo.witghSmple), 4);//测试
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

			if(g_sRaderInfo.dishInfo.state == READER_DISH_INFO_CHG)      //餐信息不变化
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

			if(!(Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue) & 0x80000000) && (Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue) >=  g_sDeviceParamenter.scoWt) && !g_nReaderGpbState)  
			{
			    pDishInfo->peice = tempValue * (pDishInfo->unitPrice) / (pDishInfo->spec);
			    g_nReaderGpbState = TRUE;
			}
			else if(!(Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue) & 0x80000000) && g_nReaderGpbState)
			{  
			    tempValue = Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue);
			    pDishInfo->peice = tempValue * (pDishInfo->unitPrice) / (pDishInfo->spec);
			}
			else
			{
				tempValue = 0;	
			    pDishInfo->peice = 0;
			}
			
			if(g_sRfidInfo.flag == RFID_TAG_OUT)
			{
				if(g_nReaderWightValue >= tempValue)
				{
					g_nReaderWightValue = tempValue;
				}	
			
			}
			
			{

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
			}
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

void Reader_Chk_KeyValue(u8 value)
{
    static u8 keyValue = 0, k = 0;
 
    if(g_sKeyValue.state == KEY_STAT_IDLE)
    {
        if(value)
        {
			k = value;
			g_sKeyValue.tick = g_nSysTick;
			g_sKeyValue.state =KEY_STAT_SAMPLE;
        }
        else
        {
			g_sKeyValue.value = 0;
        }
    }
    else if(g_sKeyValue.state == KEY_STAT_SAMPLE)
    {	
        if(g_sKeyValue.tick + KEY_LONG_SAMPLE_TIME < g_nSysTick)
        {
			if(k == value)
			{
			    k = value;
			}
			else
			{
			    g_sKeyValue.value = 0;
			}
			g_sKeyValue.state = KEY_STAT_OK;
        }
        else
        {
			g_sKeyValue.value = value;
			g_sKeyValue.state = KEY_STAT_OK;
        }
    }
    else if(g_sKeyValue.state == KEY_STAT_OK)
    {
        if(g_sKeyValue.tick + KEY_LONG_PRESS_TIME < g_nSysTick)
        {
			if(k == value)
			{
			    g_sKeyValue.value |= (k << 3);
			}
			else
			{
			    g_sKeyValue.value |= k;
			}
			g_sKeyValue.state =KEY_STAT_IDLE;	
        }
        else
        {
			g_sKeyValue.value = value;
			g_sKeyValue.state =KEY_STAT_SAMPLE; 			 
        }
    }
    
    if(keyValue != value)
    {
        keyValue = value;
        if(keyValue & KEY_SAMPLE_UP)
        {
			Lcm_DishWriteColor(LCE_TXT_ADDR_KEY_UP, LCM_FONT_COROL_BLACK);
        }
        else
        {
			Lcm_DishWriteColor(LCE_TXT_ADDR_KEY_UP, LCM_FONT_COROL_WHITE);
        }
			    
        if(keyValue & KEY_SAMPLE_MIDDLE)
        {
			Lcm_DishWriteColor(LCE_TXT_ADDR_KEY_MIDDLE, LCM_FONT_COROL_BLACK);
        }
        else
        {
			Lcm_DishWriteColor(LCE_TXT_ADDR_KEY_MIDDLE, LCM_FONT_COROL_WHITE);
        }
        if(keyValue & KEY_SAMPLE_DOWN)
        {
			Lcm_DishWriteColor(LCE_TXT_ADDR_KEY_DOWN, LCM_FONT_COROL_BLACK);
        }
        else
        {
			Lcm_DishWriteColor(LCE_TXT_ADDR_KEY_DOWN, LCM_FONT_COROL_WHITE);
        }   
    
    }
    else
    {
        keyValue = value;
    }
}




void Reader_Normal_Mode()
{
    static u8 upLink = 0; 

     if(a_CheckStateBit(g_nReaderState, READER_STAT_LCM_FRESH))
        {   
			a_ClearStateBit(g_nReaderState, READER_STAT_LCM_FRESH); 
			Reader_DisplayDish(&g_sRaderInfo.dishInfo , &g_sGpbInfo, &g_sLcmInfo);
        }
        if(a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL))
        {
			Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_WARN);
			memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
			memcpy(g_nBufTxt, "当前设备不可用", 14);
        }

        if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_IN))
        { 
			a_ClearStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_IN); 
			if((Lcm_ChkPage(LCM_PAGE_MAIN_WHITE) || Lcm_ChkPage(LCM_PAGE_INFO_WHITE)) )
			{
			    g_sDeviceRspFrame.len = Reader_Format_GetPerson(g_sReaderRfidTempInfo.uid, &g_sDeviceRspFrame, g_nRtcTime);
			    g_sReaderRfidTempInfo.tempState = RFID_TAG_IN;
			    a_SetStateBit(g_nReaderState, READER_STAT_UARTTX);
				//g_nReaderWightValue = READER_WIGHT_MAX_VALUE;//Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue);
			}
        }
        if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_OUT)) //&& (!a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL)))
        {
			memset(&g_sRaderInfo.personInfo, 0, sizeof(PERSON_INFO));
			if((Lcm_ChkPage(LCM_PAGE_MAIN_WHITE) || Lcm_ChkPage(LCM_PAGE_INFO_WHITE)))
			{
			    a_ClearStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
			    if(Lcm_ChkPage(LCM_PAGE_MAIN_WHITE))
			    {
			        if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_OUT))
			        {
						if(a_CheckStateBit(g_nReaderState, READER_STAT_MODE_NORMAL))
						{
						     a_ClearStateBit(g_nReaderState, READER_STAT_MODE_NORMAL) ;	
							// static int tempvalue = 0;
									//tempvalue = Gpb_ChkValue(g_sGpbInfo.wightValue, g_sGpbInfo.shankValue);   //测试数据，可删
						     if( !(Gpb_ChkValue(g_sGpbInfo.wightValue, g_sGpbInfo.shankValue) & 0x80000000) && Gpb_ChkValue(g_sGpbInfo.wightValue,g_sGpbInfo.shankValue) > g_sDeviceParamenter.scoWt)
						     {
								g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
								g_sSoundInfo.txBuf.data = SOUND_VOC_PUT_TAG;
								a_SetStateBit(g_nReaderState, READER_STAT_PUT_TAG);
						     }
						}
			        }
			    }
			    
			    a_ClearStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_OUT); 
			    memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
			    memcpy(g_nBufTxt, "请放入托盘", 10);
			    Lcm_SetPage(LCM_FLAG_PAGE_MAIN_CHG);
			    Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_WARN);

			    if(g_sReaderRfidTempInfo.tempState == RFID_TAG_IN)
			    {
			        g_sReaderRfidTempInfo.tempState = RFID_TAG_OUT;
			        g_sRaderInfo.dishInfo.peice = 0;
			        memset(&g_sRaderInfo.personInfo, 0, sizeof(PERSON_INFO));
															//----测试数据，可删，负值是否需要过滤？
			        if(g_nReaderWightValue >= READER_WITHT_LIMIT_VALUE && g_nReaderGpbState)
			        {
						g_sDeviceMealRspFrame.len = Reader_Format_Meal(g_sRaderInfo.dishInfo.type, g_sReaderRfidTempInfo.uid, &g_sDeviceMealRspFrame, g_nRtcTime, g_nReaderWightValue);//Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue));//RTC???
						g_nOffLineLimitTime = g_nSysTick;
						
						/*
						if(g_nReaderWightValue == READER_WIGHT_MAX_VALUE)
						{
						
							g_nOffLineLimitTime = g_nSysTick;
						}
						*/
						a_SetStateBit(g_nReaderState, READER_STAT_UARTRX);
						g_sDeviceMealRspFrame.tick = g_nSysTick;
						g_nReaderWightValue = 0;
						g_sDeviceMealRspFrame.mode = READER_RSP_NORMAL_DATA;
						
			        }
					else
					{
						g_nOffLineLimitTime = g_nSysTick;
					}
			     }
			    g_sDeviceRspFrame.succFlag =  READER_RESFRAME_PERSON_FAIL;
			}
			g_nReaderGpbState = FALSE;
        }
     
     //get_uid = 450ms
     //change_page = 900;
       
        if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
        {
			if(Lcm_ChkPage(LCM_PAGE_MAIN_WHITE))
			{
			    if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AIR)
			    {
			        Lcm_SetPage(LCM_FLAG_PAGE_INFO_CHG);
			    }
			    else if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AFTER)
			    {
			        Lcm_SetPage(LCM_FLAG_PAGE_INFO_CHG);
			        a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
			    }
			}
			else if(Lcm_ChkPage(LCM_PAGE_INFO_WHITE))
			{
			    memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
			    BOOL modeMask = FALSE, allergMask = FALSE;
			    
			    if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AIR && !a_CheckStateBit(g_nSysState, SYS_STAT_LINE_OFF))
			    {
			        if(g_sDeviceRspFrame.succFlag == READER_RESFRAME_PERSON_OK || g_sDeviceRspFrame.succFlag == READER_RESFRAME_PERSON_TOTIME)
			        {
						if(g_sRaderInfo.personInfo.bind == READER_PERSON_BIND_FAIL)
						{
						    Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_WARN);
						    g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
						    g_sSoundInfo.txBuf.data = SOUND_VOC_NO_BIND;
						    a_SetStateBit(g_nReaderState, READER_STAT_BIND_TAG);  
						}
						else
						{
						    a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
						    modeMask = TRUE;

						}     
						
						if(g_sRaderInfo.personInfo.allergy == READER_PERSON_ALLERGY)
						{
						        memcpy(g_nBufTxt, "您对该菜品过敏", 14);
						        Lcm_BackgroundCtr(LCM_BACKGROUND_STR_LDRAK, LCM_BACKGROUND_ADDR_INFO);
						        Lcm_BackgroundCtr(LCM_BACKGROUND_STR_HDRAK, LCM_BACKGROUND_ADDR_ALLERGY);
						        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_ALLERGY);
						        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_ALLERGY_BACK);
						}
						else
						{
						        Lcm_BackgroundCtr(LCM_BACKGROUND_STR_NORMAL, LCM_BACKGROUND_ADDR_INFO);
						        Lcm_BackgroundCtr(LCM_BACKGROUND_STR_NORMAL, LCM_BACKGROUND_ADDR_ALLERGY);
						        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY);
						        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY_BACK);
						        allergMask = TRUE;
						}
						
						if(allergMask && modeMask)
						{
						    a_SetStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
						}
						else
						{
						    a_ClearStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
						}
			        }
			    }
			    else
			    {
			        Lcm_BackgroundCtr(LCM_BACKGROUND_STR_NORMAL, LCM_BACKGROUND_ADDR_INFO);
			        Lcm_BackgroundCtr(LCM_BACKGROUND_STR_NORMAL, LCM_BACKGROUND_ADDR_ALLERGY);
			        memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
			        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY);
			        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY_BACK);
			        a_SetStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
			
			    }
			}
        }

        if(a_CheckStateBit(g_sReaderRfidTempInfo.clash, RFID_TAG_FAIL))
        {
			a_ClearStateBit(g_sReaderRfidTempInfo.clash, RFID_TAG_FAIL);
			a_SetStateBit(g_nReaderState, READER_STAT_CLASH_TAG);
			g_sReaderRfidTempInfo.tempState = RFID_TAG_IN;

			g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
			g_sSoundInfo.txBuf.data = SOUND_VOC_TAG_CLASH;

        }
      
    if(a_CheckStateBit(g_sDeviceMealRspFrame.state, READER_DATA_UP_OK))
    {
        a_ClearStateBit(g_sDeviceMealRspFrame.state, READER_DATA_UP_OK);
        if(Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) > 0)			     //      数据上传成功
        {
			g_sFramData.id = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);
			if(g_sFramData.id == g_sReaderOffLineInfo.index)
			{
			     memset(g_sFramData.pBuffer , 0, FRAM_OFFLINE_DATA_LEN);
			    if(Reader_WriteOffLineDatas((g_sFramData.id * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer))
			    {
			        g_sReaderOffLineInfo.readerOffLineData[g_sReaderOffLineInfo.index >> 3] &= ~(0x80 >> (g_sReaderOffLineInfo.index & 0x07));
			        Reader_WriteOffLineDataNum();
			        Reader_ReadOffLineDataNum();
			    }
			}
			else 
			{
			    memset(g_sFramData.pBuffer , 0, FRAM_OFFLINE_DATA_LEN);
			    if(Reader_WriteOffLineDatas((g_sReaderOffLineInfo.index * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer))
			    {
			        g_sReaderOffLineInfo.readerOffLineData[g_sReaderOffLineInfo.index >> 3] &= ~(0x80 >> (g_sReaderOffLineInfo.index & 0x07));
			        Reader_WriteOffLineDataNum();
			        Reader_ReadOffLineDataNum();
			    }
			}
			g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
			g_sReaderOffLineInfo.boundaryNum = g_sReaderOffLineInfo.num;
        }
        a_ClearStateBit(g_nReaderState, READER_STAT_UART_WAIT);
    }
    
    
    if(g_sDeviceMealRspFrame.tag == REDAER_UPMEALINFO_FAIL)
    {
        
        g_sDeviceMealRspFrame.tag = REDAER_UPMEALINFO_SUCC;
        if(g_sDeviceMealRspFrame.mode == READER_RSP_NORMAL_DATA)
        {
			memset(&g_sFramData, 0, sizeof(FRAM_DATA));
			g_sReaderOffLineInfo.index = Reader_Seek_Inq(g_sReaderOffLineInfo.readerOffLineData);
			g_sFramData.id = Reader_Seek_Inq(g_sReaderOffLineInfo.readerOffLineData);
			g_sReaderOffLineInfo.readerOffLineData[g_sFramData.id >> 3] |= 0x80 >> (g_sFramData.id & 0x07);
			Reader_WriteOffLineDataNum();
			Reader_ReadOffLineDataNum();
			g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
			memcpy(g_sFramData.pBuffer , FlashTempData, FRAM_OFFLINE_DATA_LEN);
			Reader_WriteOffLineDatas((g_sFramData.id * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
        
        }
    }

    
	if(a_CheckStateBit(g_nTempLink, READER_LINK_OK))
    {
		if(!a_CheckStateBit(g_nReaderState, READER_STAT_UART_WAIT))        //卡住
        {       
			if(a_CheckStateBit(g_nReaderState, READER_STAT_CHK_UPDATA))
			{
			    upLink ++;
			    a_ClearStateBit(g_nReaderState, READER_STAT_CHK_UPDATA);    
			    if(upLink == REDAER_UP_DATA_TICK)
			    {
			         upLink = 0;
			         if(Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) > 0)						        //离线数据上传-
			         {
						g_sReaderOffLineInfo.index = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);
						g_sFramData.id = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);

						Reader_ReadOffLineDatas((g_sFramData.id * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
						memcpy(&g_sDeviceMealRspFrame.buffer, &g_sFramData.pBuffer, READER_RSP_OFFLINE_DATA_LEN);
						g_sDeviceMealRspFrame.len = READER_RSP_OFFLINE_DATA_LEN;
						a_SetStateBit(g_nReaderState, READER_STAT_UARTRX);
						g_sDeviceMealRspFrame.tick = g_nSysTick;
			         	g_sDeviceMealRspFrame.mode = READER_RSP_OFFLINE_DATA;
			         }
			    }
			}
        }
        a_ClearStateBit(g_nSysState, SYS_STAT_OVER_OFFLINE);
    }
    
    if(g_nTempLink == READER_LINK_FAIL)          
    {
        if(/*Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData)*/ g_sReaderOffLineInfo.num > READER_OFFLINE_DATA_BOUNDARY)						        //离线数据超出设定值
        {
			if(g_sReaderOffLineInfo.boundaryNum < Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData))
			{
			    g_sReaderOffLineInfo.boundaryNum = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
				g_sSoundInfo.state = SOUND_STAT_TX; 
				g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
				g_sSoundInfo.txBuf.data = SOUND_VOC_OFF_LINE;
			    a_SetStateBit(g_nSysState, SYS_STAT_OVER_OFFLINE);
			}
        }
    }


}

void Reader_ResolveWitgh()
{
	static u32 tempWightSampleValue = 0;
	u32 tempWightValue = 0;
	
	tempWightValue = Gpb_GetValue(g_sGpbInfo.rxBuf.buffer);
	
	g_sGpbInfo.witghSmple = (u32)(((tempWightValue & GPB_WITGH_MASK_ALL_VALUE) / pow(10, (tempWightValue & GPB_WITGH_MASK_VALUE_POINT) >> 20)) * 1000);			        
	Witgh_CalAvg(&g_sWightTempInfo, g_sGpbInfo.witghSmple);

	if(abs(g_sGpbInfo.wightTemp - g_sWightTempInfo.avg) > READER_OVER_WIGHT_VALUE)        //1g抖动不处理,1g连续十次，恢复
	{
		g_sWightTempInfo.tick = 0;
		g_sGpbInfo.wightTemp =	g_sWightTempInfo.avg;
	}
	else
	{
		g_sWightTempInfo.tick ++;
	}

	if(g_sWightTempInfo.tick > READER_OVER_WIGHT_TICK)
	{
		g_sWightTempInfo.tick = 0;
		g_sGpbInfo.wightTemp = g_sWightTempInfo.avg;
	}
	
	if(abs(g_sWightTempInfo.avg - tempWightSampleValue) <= GPB_SHANK_WIGHT_VALUE) //防抖
	{
		if(g_sGpbInfo.shankTick >= GPB_SHANK_WIGHT_TICK)
		{
			g_sGpbInfo.shankTick = 0;
			g_sGpbInfo.shankValue =  tempWightSampleValue;
		}
		else
		{
			g_sGpbInfo.shankTick++;
		}
	}
	else
	{
		g_sGpbInfo.shankTick = 0;
		tempWightSampleValue = g_sWightTempInfo.avg;
	}
	



}
