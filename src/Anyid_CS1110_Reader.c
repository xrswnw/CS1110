#include "AnyID_CS1110_Reader.h"

const u8 READER_VERSION[READER_VERSION_SIZE]@0x08005000 = " CS1110 23092602 G230200";

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
READER_DATA g_sReaderData = {0};

int g_nReaderWightValue = 0;
char g_nBufTxt[LCM_TXT_LEN_MAX] = "未绑定托盘"; 
char g_nBufTxt1[LCM_TXT_LEN_MAX] = ""; 
char g_nAlleryBufTxt[LCM_TXT_LEN_MAX] = "您对该菜品过敏"; 

void Reader_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}


BOOL Reader_RfidInit()
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
	g_sReaderOffLineInfo.num = Reader_SeekNum(g_sReaderOffLineInfo.readerOffLineData);
	g_sReaderOffLineInfo.boundaryNum = g_sReaderOffLineInfo.num;
    g_sDeviceParamenter.reWorkMode = READER_MODE_NORMAL;
	
	Device_VoiceApoFrame(SOUND_FRAME_CMD_VOL_STR_AOP, SOUND_CNT_TIME_1S * 1.5, SOUND_REPAT_NULL, SOUND_VOICE_CTR_STRENGH, SOUND_CTR_VOC_STR);
    Reader_FontChgCorol(g_sDeviceParamenter.uiMode);
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
        g_sDeviceParamenter.ip[0] = READER_DEFAULT_PARAMETER_IP;
        g_sDeviceParamenter.ip[1] = READER_DEFAULT_PARAMETER_IP;
        g_sDeviceParamenter.ip[2] = READER_DEFAULT_PARAMETER_IP;
        g_sDeviceParamenter.ip[3] = READER_DEFAULT_PARAMETER_IP;
        g_sDeviceParamenter.rfWorkMode = READER_RFID_READ_UID;
        g_sDeviceParamenter.totalMode = READER_TOTAL_MODE_AIR;
        g_sDeviceParamenter.uiMode = READER_UI_MODE_WHITE;
        g_sDeviceParamenter.scoWt = READER_DEFAULT_PARAMETER_SOWT;
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

    g_sFramData.id = Reader_SeekId(g_sReaderOffLineInfo.readerOffLineData);
    g_sReaderOffLineInfo.num = Reader_SeekNum(g_sReaderOffLineInfo.readerOffLineData);
    Reader_ReadOffLineDatas(g_sFramData.id * FLASE_PAGE_SIZE + FRAM_OFF_LINE_ADDR, FLASH_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
    

}

u16 Reader_SeekId(u8 *pBuffer)          //扫描离线数据
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

u16 Reader_SeekNum(u8 *pBuffer)          //扫描离线数据
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

u16 Reader_SeekInq(u8 *pBuffer)          //扫描离线数据
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
				
				if(g_nReaderWightValue != READER_WIGHT_MAX_VALUE)
				{
					g_nReaderWightValue = READER_WIGHT_MAX_VALUE;
				}
				
			}
			else if((pRfidInfo->okTick >= linkTick))						  //标签重叠，由多便签变化为单标签，动态增加，后续归为
			{
			     pRfidInfo->state = RFID_TAG_KEEP;
							
				
				 if(memcmp(pRfidInfo->uid, tempUid, ISO15693_SIZE_UID))
				{
					 pRfidInfo->clashTick ++;
				}
				else
				{
					pRfidInfo->clashTick = 0;
				}
				if(pRfidInfo->clashTick >= RFID_ONE_TAG_TIME)
				{
					pRfidInfo->okTick = 0;
			        pRfidInfo->errTick += READER_RFID_MOVE_TIME;       //数据传输问题
			        pRfidInfo->state = RFID_TAG_OUT;
					g_sRfidInfo.flag = RFID_TAG_OUT;
				}
			}
        }
        else
        {
			//多餐盘
			tempState = TRUE;
			if(pRfidInfo->clashTicks >= RFID_ONE_TAG_TIME)
			{
				pRfidInfo->clashTicks = 0;
				pRfidInfo->okTick = 0;
				g_sRfidInfo.flag = RFID_TAG_OUT;
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
		pRfidInfo->clashTicks = 0;
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
    sprintf(Buf, "%d", g_sReaderRfidTempInfo.num);   
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_BUTTON_2, LCM_TXT_TEST_BUTTON_LEN, Buf, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
	Lcm_DishWriteTxt(LCM_TXT_ADDR_SOFT_VERSION, READER_SOFT_VERSON_LEN, (char *)&READER_VERSION + 1, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
	Lcm_DishWriteTxt(LCM_TXT_ADDR_RFID_SOFT_VERSION, READER_SOFT_VERSON_LEN, (char *)g_sRfidInfo.verSion, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
    sprintf(Buf, "%d", g_sReaderRfidTempInfo.allTick );    
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_2, LCM_TXT_TEST_LEN, Buf, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
	//memset(Buf, 0, LCM_TXT_LEN_MAX);
	sprintf(Buf, "%.2d.%.2d%c", percentage / 100, percentage % 100, 0x25);
    Lcm_DishWriteTxt(LCM_TXT_ADDR_TEST_3, LCM_TXT_TEST_LEN - 1 /*Reader_SearchCode(LCM_TXT_TEST_LEN, 0x25, Buf)*/, Buf, LCM_DISH_TX_MODE_LEFT, LCM_TXT_OFFSET_NULL);
    
    return state;
    
}



u8 Reader_SearchCode(u8 lenth, u8 code, char *pBuf)
{
	u8 len = 0;
	
	for(len = 0; len < lenth; len++)
	{
		if(pBuf[len] == code)
		{
			break;
		}
	}
	
	return ++len;

}


void Reader_ChgStat(u8 lineState)																								    
{
    static u8 state = 0xFF;
    char Buf[LCM_TXT_LEN_MAX];

    if(state != lineState)
    {
        state = lineState;
        if(state == RFID_LINK_FAIL)
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


u8 Reader_FormatGetPerson(u8 *puid,  READER_RSPFRAME *pOpResult, u32 rtc)
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


u8 Reader_FormatMeal(u8 type,u8 *puid, READER_RSPFRAME *pOpResult, u32 rtc, u32 witgh)
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


u8 Reader_FormatHeart( READER_RSPFRAME *pOpResult, u32 rtc,u32 witgh)
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

u8 Reader_FormatCfg(u8 mode, READER_RSPFRAME *pOpResult, READER_DEVICE_PARAMETER *pParameter)
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
        
			Lcm_DishWriteTxt(LCM_TXT_ADDR_ARRERGY, LCM_TIP_TXT_SIZE, g_nAlleryBufTxt, LCM_DISH_TX_MODE_MID, 0);
			Lcm_DishWriteTxt(LCM_TXT_ADDR_LINK_STATE, 8, g_nBufTxt1, LCM_DISH_TX_MODE_MID, 0);
/*
			if(!((int)g_sGpbInfo.wightValue(Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue) & GPB_WITGH_MASK_DOWN_VALUE)) && 
			   ((int)Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue) >=  (int)g_sDeviceParamenter.scoWt) && 
				   !g_nReaderGpbState)
*/
			if(((int)g_sGpbInfo.wightValue - (int)g_sGpbInfo.wightTemp > (int)g_sDeviceParamenter.scoWt) && !g_nReaderGpbState)
			{
			    pDishInfo->peice = tempValue * (pDishInfo->unitPrice) / (pDishInfo->spec);
			    g_nReaderGpbState = TRUE;
			}
			else if(((int)g_sGpbInfo.wightValue - (int)g_sGpbInfo.wightTemp > 0) && g_nReaderGpbState)
			{  
			    tempValue = Gpb_ChkValue(g_sGpbInfo.wightTemp, g_sGpbInfo.wightValue);
			    pDishInfo->peice = tempValue * (pDishInfo->unitPrice) / (pDishInfo->spec);
				if(g_sRfidInfo.flag == RFID_TAG_OUT)
				{
					if(g_nReaderWightValue >= tempValue)
					{
						g_nReaderWightValue = tempValue;						//取过程最小值
					}	
				}
			}
			else
			{
				tempValue = 0;	
			    pDishInfo->peice = 0;
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

void Reader_FontChgCorol(u8 background)
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
        Lcm_DishWriteColor(LCM_TXT_ADDR_LINK_INFO_COLOR, LCM_FONT_COROL_WHITE);
    }
    Lcm_SelectPage(g_sLcmInfo.page + g_sDeviceParamenter.uiMode);
}


void Reader_ChkKeyValue(u8 value)
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




void Reader_NormalMode()
{
	if(a_CheckStateBit(g_nReaderState, READER_STAT_LCM_FRESH))
	{   
		a_ClearStateBit(g_nReaderState, READER_STAT_LCM_FRESH); 
		Reader_DisplayDish(&g_sRaderInfo.dishInfo , &g_sGpbInfo, &g_sLcmInfo);
	}
	if(a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL))
	{
		Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_WARN);
		if(memcmp(g_nBufTxt, &LCM_TXT_DEVICE_ERR, LCM_TXT_LEN_MAX))
		{
			memcpy(g_nBufTxt, &LCM_TXT_DEVICE_ERR, LCM_TXT_LEN_MAX);
		}
	}

     if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_IN))
     { 
		if((Lcm_ChkPage(LCM_PAGE_MAIN_WHITE) || Lcm_ChkPage(LCM_PAGE_INFO_WHITE)))
		{
			if(g_sReaderRfidTempInfo.tempState != RFID_TAG_IN)
			{
				g_sDeviceRspFrame.len = Reader_FormatGetPerson(g_sReaderRfidTempInfo.uid, &g_sDeviceRspFrame, g_nRtcTime);
				Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
				g_sReaderRfidTempInfo.tempState = RFID_TAG_IN;
				g_sRaderInfo.personInfo.flag = READER_PERSON_BIND_MASK_FAIL;
			}
		}
     }
     if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_OUT))
     {
		 if((Lcm_ChkPage(LCM_PAGE_MAIN_WHITE) || Lcm_ChkPage(LCM_PAGE_INFO_WHITE)))
		 {
			 a_ClearStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
			 if(Lcm_ChkPage(LCM_PAGE_MAIN_WHITE))
			 {
				 if(a_CheckStateBit(g_nReaderState, READER_STAT_MODE_NORMAL))				// 55ms判断一次
				 {
					 a_ClearStateBit(g_nReaderState, READER_STAT_MODE_NORMAL) ;	
				   //重量减少，且减少值大于勺重*/
					if(g_sDeviceParamenter.scoWt != READER_DEFAULT_PARAMETER_SOWT)
					{
						 //Sys_LedBlueOn();
						 g_sDeviceParamenter.scoWt = READER_DEFAULT_PARAMETER_SOWT;
					
					}
    
					 if(((int)(g_sGpbInfo.shankValue - g_sGpbInfo.wightTemp) > (int)g_sDeviceParamenter.scoWt) && !g_sGpbInfo.stableFlag)
					 {	
					 	if(g_sSoundInfo.txBuf.op[g_sSoundInfo.txBuf.index + 1] != SOUND_VOICE_PUT_TAG)
					 	{
					 		Device_VoiceApoFrame(SOUND_FRAME_CMD_APPOINT_FOLDER, SOUND_CNT_TIME_1S * 1.5, SOUND_REPAT_NULL, SOUND_VOICE_PUT_TAG, SOUND_VOC_PUT_TAG);
					 	}
/*
						 if(g_sGpbInfo.linitTick >= GPB_WIGHTOVER_LINIT_TICK)
						 {
							 g_sGpbInfo.linitTick = 0;
							 Device_VoiceApoFrame(SOUND_FRAME_CMD_APPOINT_FOLDER, SOUND_CNT_TIME_1S * 1.5, SOUND_REPAT_NULL, SOUND_VOICE_PUT_TAG, SOUND_VOC_PUT_TAG);
						 }
						 else
						 {
						 	g_sGpbInfo.linitTick ++;
						 }
*/
					 }
					 else
					 {
						 						 //a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
					 	g_sGpbInfo.linitTick = 0;
					 }
					
				 }
				 else
				 {
				 
				 }
			 }
			
			if(memcmp(g_nBufTxt, &LCM_TXT_PUT_TAG, LCM_TXT_LEN_MAX) ||
			   Lcm_ChkPage(LCM_PAGE_INFO_WHITE)
			   )
			{
				memcpy(g_nBufTxt, &LCM_TXT_PUT_TAG, LCM_TXT_LEN_MAX);
				Lcm_SetPage(LCM_FLAG_PAGE_MAIN_CHG);
				Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_WARN);
			}

			if(g_sReaderRfidTempInfo.tempState == RFID_TAG_IN)
			{
				memset(&g_sRaderInfo.personInfo, 0, sizeof(PERSON_INFO));
				g_sReaderRfidTempInfo.tempState = RFID_TAG_OUT;
				g_sRaderInfo.dishInfo.peice = 0;
				Reader_DisplayDish(&g_sRaderInfo.dishInfo , &g_sGpbInfo, &g_sLcmInfo);		
				if(g_nReaderWightValue >= READER_WITHT_LIMIT_VALUE && g_nReaderGpbState)      //取用量大于勺重，启用
				{
					g_sDeviceRspFrame.len = Reader_FormatMeal(g_sRaderInfo.dishInfo.type, g_sReaderRfidTempInfo.uid, &g_sDeviceRspFrame, g_nRtcTime, g_nReaderWightValue);
					g_nOffLineLimitTime = g_nSysTick;											//取餐记录时间更新，判定最后一条数据上传时间
					Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
				}
				else
				{
					g_nOffLineLimitTime = g_nSysTick;           //数据监控
				}
				g_sGpbInfo.shankValue = g_sGpbInfo.wightTemp;  //Tag离开后，刷新稳定值
				g_nReaderGpbState = FALSE;   //起重开关
			}
		 }
			
	}
     
	if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
	{
		if(Lcm_ChkPage(LCM_PAGE_MAIN_WHITE))
		{
			if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AFTER)
			{
				a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
			}
			Lcm_SetPage(LCM_FLAG_PAGE_INFO_CHG);
		}
		else if(Lcm_ChkPage(LCM_PAGE_INFO_WHITE))
		{
			
			BOOL modeMask = FALSE, allergMask = FALSE;
			
			memset(g_nAlleryBufTxt,0, LCM_INFO_TXT_SIZE);		
			if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AIR && !a_CheckStateBit(g_nSysState, SYS_STAT_LINE_OFF))
			{
				if(g_sRaderInfo.personInfo.successFlag == READER_PERSON_INFO_RSP_OK || 
				g_sRaderInfo.personInfo.successFlag == READER_PERSON_INFO_RSP_OP)
				{
					if(g_sRaderInfo.personInfo.bind == READER_PERSON_BIND_FAIL)
					{
						Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_WARN);
						a_SetStateBit(g_nReaderState, READER_STAT_BIND_TAG); 
						if(a_CheckStateBit(g_nReaderState, READER_STAT_CHK_PERSON_INFO))
						{
							a_ClearStateBit(g_nReaderState, READER_STAT_CHK_PERSON_INFO);
							
							if(g_sRaderInfo.personInfo.limitTick > READER_LIMIT_RE_PERSONINFO)
							{
								g_sRaderInfo.personInfo.limitTick = 0;
								Device_VoiceApoFrame(SOUND_FRAME_CMD_APPOINT_FOLDER, SOUND_CNT_TIME_1S * 1.5, SOUND_REPAT_NULL, SOUND_VOICE_BIND_NULL, SOUND_VOC_NO_BIND);
							}
							else
							{
								g_sRaderInfo.personInfo.limitTick ++;
							}
						}
						if(g_sRaderInfo.personInfo.flag == READER_PERSON_BIND_MASK_OK || g_sRaderInfo.personInfo.flag == READER_PERSON_BIND_MASK_OP_TIME)
						{
							g_sRaderInfo.personInfo.flag = READER_PERSON_BIND_MASK_FAIL;
							Device_VoiceApoFrame(SOUND_FRAME_CMD_APPOINT_FOLDER, SOUND_CNT_TIME_1S * 1.5, SOUND_REPAT_NULL, SOUND_VOICE_BIND_NULL, SOUND_VOC_NO_BIND);
						}
					}
					else
					{
						g_sRaderInfo.personInfo.limitTick = 0;
						a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
						modeMask = TRUE;
					}       
					if(g_sRaderInfo.personInfo.allergy == READER_PERSON_ALLERGY)
					{
						if(memcmp(g_nAlleryBufTxt, &LCM_TXT_ALLERY, LCM_TXT_LEN_MAX))
						{
							memcpy(g_nAlleryBufTxt, &LCM_TXT_ALLERY, LCM_TXT_LEN_MAX);
						}
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
		Device_VoiceApoFrame(SOUND_FRAME_CMD_APPOINT_FOLDER, SOUND_CNT_TIME_1S * 1.5, SOUND_REPAT_NULL, SOUND_VOICE_CLASH_TAG, SOUND_VOC_TAG_CLASH);
	}
}


void Reader_ResolveWitgh()
{
	static int tempWightSampleValue = 0, tempWightValue = 0;
	
	if((Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & GPB_WITGH_MASK_VALUE) == GPB_WITGH_MASK_VALUE)
	{
		tempWightValue = (int)-(((Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & GPB_WITGH_MASK_VALUE_ABS) / 
								 pow(10, (Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & GPB_WITGH_MASK_VALUE_POINT) >> 20)) * 1000);        
	}
	else
	{
		tempWightValue = (int)(((Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & GPB_WITGH_MASK_VALUE_ABS) / 
								pow(10, (Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & GPB_WITGH_MASK_VALUE_POINT) >> 20)) * 1000);        
	}

	if(abs(tempWightValue - g_sWightTempInfo.avg) < GPB_WITGH_MAX_VAR_DEFF)       //每次拉去重量变化的最大值的2.5倍阈值，如果超过这个限制，jiang
	{
		Witgh_CalAvg(&g_sWightTempInfo, Gpb_GetValue(g_sGpbInfo.rxBuf.buffer));
		g_sGpbInfo.wightTemp = g_sWightTempInfo.avg;
		if(abs(g_sWightTempInfo.avg - tempWightSampleValue) <= GPB_SHANK_WIGHT_VALUE) //防抖
		{
			if(g_sGpbInfo.shankTick >= GPB_SHANK_WIGHT_TICK)
			{
				g_sGpbInfo.shankTick = 0;
				g_sGpbInfo.stableFlag = TRUE;
				g_sGpbInfo.shankValue =  g_sWightTempInfo.avg;
				a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
			}
			else
			{
				g_sGpbInfo.shankTick++;
			}
		}
		else
		{
			g_sGpbInfo.stableFlag = FALSE; 
			tempWightSampleValue  = g_sWightTempInfo.avg;
			g_sGpbInfo.shankTick = 0;
		}
	}else
	{			//确实出现此情况，是否替换？？？
	
	
	}
}



//

u16 Reader_ProcessUartFrames(u8 *pFrame, u16 len)
{       
    u8 cmd = 0;
    u16 destAddr = 0;
    u16 paramsLen = 0;
    u32 rtc = 0;
    
    
    destAddr = *((u16 *)(pFrame + UART_FRAME_POS_DESTADDR));
    memcpy(&rtc, pFrame + READER_MEAL_RTC_LEN - 1, 4);
    if((destAddr != READER_FRAME_BROADCAST_ADDR) && (destAddr != g_sDeviceParamenter.addr))
    {
        return 0;
    }
    if(*(pFrame + UART_FRAME_POS_CMD) == UART_FRAME_RESPONSE_FLAG)
    {
        cmd = *(pFrame + UART_FRAME_POS_CMD + 1);
    }
    else
    {
        cmd = *(pFrame + UART_FRAME_POS_CMD);
    }

    g_sDeviceRspFrame.destAddr = *((u16 *)(pFrame + UART_FRAME_POS_SRCADDR));
    g_sDeviceRspFrame.len = 0;
    g_sDeviceRspFrame.cmd = cmd;
    g_sDeviceRspFrame.flag = UART_FRAME_FLAG_OK;
    g_sDeviceRspFrame.err = UART_FRAME_RSP_NOERR;
    
    
    if(cmd != READER_CMD_CHG_IP)        //断线情况下，网络模块和主板交互不刷新Tick
    {
        g_nTempLink = READER_LINK_OK;           
        g_nTickLink = g_nSysTick;
		if(cmd != READER_CMD_GET_PERSON_INFO && cmd != READER_CMD_GET_MEAL_INFO)
		{
			g_sReaderData.priority = READER_DATA_PRIORITY_HIGH;
		}
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
				//Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
			}
			break;
        case READER_CMD_GET_VERSION:
            if(paramsLen == 0)
            {
				g_sDeviceRspFrame.len = Device_ResponseFrame((u8 *)READER_VERSION, READER_VERSION_SIZE, &g_sDeviceRspFrame); 
				//Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
			}
			break;
        case READER_CMD_SET_CFG:
			if(paramsLen == READER_CMD_FRAME_LEN_SET_CFG)
			{   
				u8 ioMode = 0,totalMode = 0;
				u32 scoWt = 0;
                            
				ioMode = *(pFrame+READER_SET_CFG + 2);
				totalMode = *(pFrame+READER_SET_CFG + 3);
				memcpy(&scoWt, pFrame + READER_SET_CFG + 4, 4);
                            
				if(ioMode != g_sDeviceParamenter.uiMode || totalMode != g_sDeviceParamenter.totalMode || scoWt != g_sDeviceParamenter.scoWt)
				{
					if(((ioMode == READER_UI_MODE_WHITE) || (ioMode == READER_UI_MODE_BLACK)) && 
					((totalMode == READER_TOTAL_MODE_AIR) || (totalMode == READER_TOTAL_MODE_AFTER)) &&
					scoWt >= READER_WITHT_SCOTW_MIN)
					{
						g_sDeviceParamenter.rfu1 = *(pFrame+READER_SET_CFG + 1);
						g_sDeviceParamenter.uiMode = ioMode;    //暂时锁死
						g_sDeviceParamenter.totalMode = totalMode;
						//g_sDeviceParamenter.scoWt = scoWt;			//暂时固定
						Reader_WriteDeviceParamenter();
						Reader_FontChgCorol(g_sDeviceParamenter.uiMode);
					}
					else
					{
						g_sDeviceRspFrame.flag = READER_FRAME_FLAG_FAIL;
						g_sDeviceRspFrame.err = READER_FRAME_INFO_FAIL;
					}
				}
				g_sDeviceRspFrame.len = Reader_FormatCfg(0, &g_sDeviceRspFrame, &g_sDeviceParamenter);//Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame); 
				//Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
			}
        case READER_CMD_GET_CFG:
			if(paramsLen == 0)
			{
				g_sDeviceRspFrame.len = Reader_FormatCfg(1, &g_sDeviceRspFrame, &g_sDeviceParamenter); 
				//Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
			}
			break;
        case READER_CMD_GET_HEART:
			if(paramsLen > READER_CMD_FRAME_LEN_MEAL)
			{
			    u32 rtcTime = 0;
			    if(!a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
			    {
					u8 mealNameLen = 0,tagNameLen = 0;;
                                
					mealNameLen = *(pFrame + READER_MEAL_NAME_LEN);
					tagNameLen = *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1);
					if(mealNameLen > READER_MEAL_NAME_LEN_MIN && mealNameLen <= READER_MEAL_NAME_LEN_MAX && tagNameLen <= READER_MEAL_TAG_NAME_LEN_MIN)
					{
						memset(&g_sDishTempInfo, 0, sizeof(DISH_INFO));
						memcpy(&g_sDishTempInfo.dishName, pFrame + READER_MEAL_NAME_LEN, *(pFrame + READER_MEAL_NAME_LEN) + 1);
						memcpy(&g_sDishTempInfo.unitPrice,pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN)+ 1 + *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) + 1, 4);
						memcpy(&g_sDishTempInfo.spec,pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN)+ 1 + *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) + 1 + 4, 2);
							
													
						g_sDishTempInfo.tagNum = Reader_Lable(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1, &g_sDishTempInfo);
						g_sDishTempInfo.type = *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN)+ 1 + *(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1) + 1 + 4 + 2);
						
						if((memcmp(&g_sRaderInfo.dishInfo.dishName, &g_sDishTempInfo.dishName, NAME_NUM)) || 
						(memcmp(&g_sRaderInfo.dishInfo.tagInfo, &g_sDishTempInfo.tagInfo, sizeof(DISH_TAG_INFO))) || 
						(memcmp(&g_sRaderInfo.dishInfo.unitPrice, &g_sDishTempInfo.unitPrice, sizeof(g_sDishTempInfo.unitPrice))) ||
						(memcmp(&g_sRaderInfo.dishInfo.spec, &g_sDishTempInfo.spec, sizeof(g_sDishTempInfo.spec))) || 
						(memcmp(&g_sRaderInfo.dishInfo.type, &g_sDishTempInfo.type, sizeof(g_sDishTempInfo.type))))
						{
							memcpy(&g_sRaderInfo.dishInfo.dishName, &g_sDishTempInfo.dishName, NAME_NUM);
							memcpy(&g_sRaderInfo.dishInfo.unitPrice, &g_sDishTempInfo.unitPrice, sizeof(g_sDishTempInfo.unitPrice));
							memcpy(&g_sRaderInfo.dishInfo.spec,  &g_sDishTempInfo.spec, sizeof(g_sDishTempInfo.spec));
							memcpy(&g_sRaderInfo.dishInfo.type, &g_sDishTempInfo.type, sizeof(g_sDishTempInfo.type));
							g_sRaderInfo.dishInfo.tagNum = Reader_Lable(pFrame + READER_MEAL_NAME_LEN + *(pFrame + READER_MEAL_NAME_LEN) + 1, &g_sRaderInfo.dishInfo);
							g_sRaderInfo.dishInfo.state = READER_DISH_INFO_CHG;
						}

						//memcpy(&rtcTime, pFrame + READER_MEAL_RTC_POS, 4);
						//RTC_SetTime(rtcTime);                                   //校准服务器时间
													
						if((Lcm_ChkPage(LCM_FLAG_PAGE_NULL_CHG) || 
							Lcm_ChkPage(LCM_FLAG_PAGE_NULL_CHG + READER_UI_MODE_BLACK)) && 
							! Lcm_ChkPage(LCM_FLAG_PAGE_TEST))
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
			   	
			    memcpy(&rtcTime, pFrame + READER_MEAL_RTC_POS, 4);
			    RTC_SetTime(rtcTime);																			//校准服务器时间
			    g_sDeviceRspFrame.len = Reader_FormatHeart(&g_sDeviceRspFrame, rtcTime, g_sGpbInfo.wightTemp);   //重量数据代填
					//Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
			}
			break;
        case READER_CMD_GET_PERSON_INFO:
			if(paramsLen == READER_CMD_FRAME_LEN_PERSON)
			{
			    if(!memcmp(pFrame + READER_PERSON_RTC_LEN, &(g_sReaderData.data[g_sReaderData.index].rtcTick), 4)) 
			    {
			        memcpy(&g_sRaderInfo.personInfo.money, pFrame + READER_PERSON_UID_LEN + 12, 4);
			        g_sRaderInfo.personInfo.state = READER_RESFRAME_PERSON_OK;
			        g_sRaderInfo.total = *(pFrame + READER_PERSON_MONEY_LEN);
			        g_sRaderInfo.personInfo.allergy = *(pFrame + READER_PERSON_MONEY_LEN);
			        g_sRaderInfo.personInfo.bind = *(pFrame + READER_PERSON_MONEY_LEN + 1);
                                
					g_sRaderInfo.personInfo.successFlag = READER_PERSON_INFO_RSP_OK;  
					g_sRaderInfo.personInfo.flag = READER_PERSON_BIND_MASK_OK;//服务器返回数据校验上传RTC通过
			    }
			    else
			    {
					g_sRaderInfo.personInfo.successFlag = READER_PERSON_INFO_RSP_FAIL;              //失败
			    }
			}
			break;
        case READER_CMD_GET_MEAL_INFO:
			if(paramsLen > 1)
			{
				if(*(pFrame + READER_PERSON_UID_LEN + 12) == READER_RES_OK  &&  
				!memcmp(pFrame + READER_PERSON_RTC_LEN, &(g_sReaderData.data[g_sReaderData.index].rtcTick), 4))          //正常报文
				{
					g_sDeviceRspFrame.len = READER_CMD_FRAME_LEN_RSP_OK;
				}
				else if(*(pFrame + READER_PERSON_UID_LEN + 12) == READER_RES_OK  &&  
				!memcmp(pFrame + READER_PERSON_RTC_LEN, g_sReaderData.data[g_sReaderData.index].buffer +  READER_PERSON_RTC_LEN -1, 4))  //离线报文
				{
					g_sDeviceRspFrame.len = READER_CMD_FRAME_LEN_RSP_OK;
					g_sReaderData.sign = READER_DATA_SIGN_UP_OK;
					Reader_OLInfoHanld();
				}
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
			if(*(pFrame + UART_FRAME_POS_PAR) == DEVICE_ADDR_READER)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_RFID;
			}
			else if(*(pFrame + UART_FRAME_POS_PAR) == DEVICE_ADDR_WIGHT)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_WIGHT;
			}
			else if(*(pFrame + UART_FRAME_POS_PAR) == DEVICE_ADDR_LCM)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_LCM;
			}
			else if(*(pFrame + UART_FRAME_POS_PAR) == DEVICE_ADDR_NULL)
			{
				g_sDeviceRspFrame.deviceFlag = READER_CMD_DRU_RESET;
			}
			g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
			//Reader_UartAddList(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
        }
      break;
    
    }
     if(g_sDeviceRspFrame.len == 0 && (cmd != READER_CMD_CHG_IP))
    {
        g_sDeviceRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
        g_sDeviceRspFrame.err = READER_OPTAG_RESPONSE_PARERR;
        g_sDeviceRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDeviceRspFrame);
    }
    
    return g_sDeviceRspFrame.len;//
}


void Reader_OLInfoHanld()
{
    READER_OFFLINE_INFO *pIdList = NULL;
    FRAM_DATA *pIdData = NULL;
    
    pIdList = &g_sReaderOffLineInfo;
    pIdData = &g_sFramData;
    
	if(g_sReaderData.priority == READER_DATA_PRIORITY_LOW)
    {
		if(g_sReaderData.sign == READER_DATA_SIGN_IDLE)
		{
        //pIdList->num = Reader_SeekNum(pIdList->readerOffLineData);                                  //拉取离线数目
			if(pIdList->num)
			{
				pIdList->index = Reader_SeekId(pIdList->readerOffLineData);
				g_sReaderData.sign = READER_DATA_SIGN_BUSY;
				Reader_ReadOffLineDatas((pIdList->index * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, 
										sizeof(FRAM_DATA), (u8 *)pIdData);
				Reader_UartAddList(pIdData->pBuffer, READER_RSP_OFFLINE_DATA_LEN);                      //此版本离线信息长度固定
			}
		}
    }
    
    if(g_sReaderData.sign == READER_DATA_SIGN_UP_OK)
    { 
		if(pIdList->index == pIdData ->id)
		{
			memset(pIdData->pBuffer , 0, FRAM_OFFLINE_DATA_LEN);
			g_sReaderData.sign = READER_DATA_SIGN_IDLE;
			if(Reader_WriteOffLineDatas((pIdList->index * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, 
										sizeof(FRAM_DATA), (u8 *)pIdData))
			{
				pIdList->readerOffLineData[pIdList->index >> 3] &= ~(0x80 >> (pIdList->index & 0x07));
				Reader_WriteOffLineDataNum();
				Reader_ReadOffLineDataNum();
				pIdList->num = Reader_SeekNum(pIdList->readerOffLineData);
			}
			else
			{
			  //EEPROM操作失败
				 g_sReaderData.sign = READER_DATA_SIGN_BUSY;
			}
		}
		else 
		{
			 g_sReaderData.sign = READER_DATA_SIGN_BUSY;
		}
        //g_sReaderOffLineInfo.boundaryNum = g_sReaderOffLineInfo.num;						//增加一条离线记录报警一次
    }
    else if(g_sReaderData.sign == READER_DATA_SIGN_DATA_ADD)
    {
        g_sReaderData.sign = READER_DATA_SIGN_IDLE;
		if(g_sReaderData.data[g_sReaderData.index].len <= FRAM_OFFLINE_DATA_LEN && 
		   pIdList->num <= (READER_OFF_LINE_DATA_NUM * 8))
		{
			if(memcmp(g_sReaderData.data[g_sReaderData.index].buffer, pIdData->pBuffer, 
					  g_sReaderData.data[g_sReaderData.index].len))
			{
				memset(&g_sFramData, 0, sizeof(FRAM_DATA));
				
				pIdList->id = Reader_SeekInq(pIdList->readerOffLineData);
				pIdList->readerOffLineData[pIdList->id >> 3] |= 0x80 >> (pIdList->id & 0x07);
				g_sReaderData.sign = READER_DATA_SIGN_IDLE;
				Reader_WriteOffLineDataNum();
				Reader_ReadOffLineDataNum();
				pIdData->len = g_sReaderData.data[g_sReaderData.index].len;
				pIdData ->id = pIdList->id;
				memcpy(pIdData->pBuffer, g_sReaderData.data[g_sReaderData.index].buffer, g_sReaderData.data[g_sReaderData.index].len);
				Reader_WriteOffLineDatas((pIdData->id * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, 
										 sizeof(FRAM_DATA), (u8 *)pIdData);
				pIdList->num = Reader_SeekNum(pIdList->readerOffLineData);
			}
			else
			{
				 g_sReaderData.sign = READER_DATA_SIGN_BUSY;
			}
		}
    
    }


}



//


