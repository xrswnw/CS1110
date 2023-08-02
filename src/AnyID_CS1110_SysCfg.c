#include "AnyID_CS1110_SysCfg.h"

u32 g_nSysState = SYS_STAT_IDLE;

u32 g_nSysGLed = 0;
u32 g_nSysBLed = 0;


void Sys_Delayms(u32 n)
{
    //72MHZ
    n *= 0x6000;
    n++;
    while(n--);
}



void Sys_CfgClock(void)
{
    ErrorStatus HSEStartUpStatus;

    RCC_DeInit();
    //Enable HSE
    RCC_HSEConfig(RCC_HSE_ON);

    //Wait till HSE is ready
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {

        //HCLK = SYSCLK = 72.0M
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        //PCLK2 = HCLK = 72.0M
        RCC_PCLK2Config(RCC_HCLK_Div1);

        //PCLK1 = HCLK/2 = 33.9M
        RCC_PCLK1Config(RCC_HCLK_Div2);

        //ADCCLK = PCLK2/2
        RCC_ADCCLKConfig(RCC_PCLK2_Div8);

        // Select USBCLK source 72 / 1.5 = 48M
        RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

        //Flash 2 wait state
        FLASH_SetLatency(FLASH_Latency_2);

        //Enable Prefetch Buffer
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        //PLLCLK = 12.00MHz * 10 = 120 MHz
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_10);    //PLL在最后设置

        //Enable PLL
        RCC_PLLCmd(ENABLE);

        //Wait till PLL is ready
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        //Select PLL as system clock source
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        //Wait till PLL is used as system clock source
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
}

void Sys_CfgPeriphClk(FunctionalState state)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
            	       RCC_APB2Periph_GPIOB |
            	       RCC_APB2Periph_GPIOC |
            	       RCC_APB2Periph_AFIO  |
            	       RCC_APB2Periph_SPI1  |
            	       RCC_APB2Periph_USART1|
            	       RCC_APB2Periph_GPIOD , state);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 |
            	       RCC_APB1Periph_USART3 |
            	       RCC_APB1Periph_UART5 |
            	       RCC_APB1Periph_UART4  |
            	       RCC_APB1Periph_SPI2   |
            	       RCC_APB1Periph_PWR    |
            	       RCC_APB1Periph_BKP , state);

}

void Sys_CfgNVIC(void)
{
    //NVIC_InitTypeDef NVIC_InitStructure;
#ifdef  VECT_TAB_RAM
    //Set the Vector Table base location at 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  //VECT_TAB_FLASH
    //Set the Vector Table base location at 0x08000000
#ifdef _ANYID_BOOT_GD32_
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif
#endif

    //Configure the Priority Group to 2 bits
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}


const PORT_INF SYS_LED_RED   = {GPIOC, GPIO_Pin_0};
const PORT_INF SYS_LED_BLUE  = {GPIOC, GPIO_Pin_1};
const PORT_INF SYS_LED_GREEN = {GPIOC, GPIO_Pin_2};

void Sys_CtrlIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
     
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = SYS_LED_RED.Pin;
    GPIO_Init(SYS_LED_RED.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SYS_LED_GREEN.Pin;
    GPIO_Init(SYS_LED_GREEN.Port, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin = SYS_LED_BLUE.Pin;
    GPIO_Init(SYS_LED_BLUE.Port, &GPIO_InitStructure);


}


void Sys_Init(void)
{
    Sys_CfgClock();
    Sys_CfgNVIC();
    Sys_CfgPeriphClk(ENABLE);
    Sys_CtrlIOInit();

    Key_Init();
    Sys_LedRedOn();
    Sys_Delayms(500); 
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    Sys_LedGreenOn();
    Sys_Delayms(500); 
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    Lcm_Init();
    Sys_LedBlueOn();
    Sys_Delayms(500); 
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    FRam_InitInterface();
    Flash_InitInterface();
    Flash_Init();

    //Reader_WriteOffLineDataNum();
    Reader_ReadOffLineDataNum();
    Rfid_Init();
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    
    Reader_Init();
    Sound_Init();
    GPB_Init();
    Sys_LedRedOff();
    Uart_Init();
    Sys_LedGreenOff();
    RTC_Init();
    RTC_ConfigInt(ENABLE);
    Sys_LedBlueOff();
   
    STick_InitSysTick();
    
    Reader_DisplayIp(&g_sDeviceParamenter);
    Sys_EnableInt();
}

void Sys_LedTask(void)
{
    if(a_CheckStateBit(g_nSysState, SYS_STAT_RUNLED))
    {
        u32 static ledTimers = 0;
        ledTimers++;
        a_ClearStateBit(g_nSysState, SYS_STAT_RUNLED);
        
        if(!a_CheckStateBit(g_sDeviceParamenter.reWorkMode, READER_MODE_TEST))
        {  
            if(a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL) || 
               a_CheckStateBit(g_nReaderState, READER_STAT_RFID_FAIL) || 
                 a_CheckStateBit(g_nSysState, SYS_STAT_LINE_OFF))
            {
                 Sys_LedRedOn();
            }
            else
            {
                Sys_LedRedOff();
            } 
            
            
            if(a_CheckStateBit(g_nReaderState, READER_STAT_READER_MARK_OK))
            {
                 Sys_LedGreenOn();
            }
            else
            {
                Sys_LedGreenOff();
            } 
            
            if(a_CheckStateBit(g_nReaderState, READER_STAT_WIGHT_ZERO_LOADING))
            {
                 Sys_LedBlueOn();
            }
            else
            {
                Sys_LedBlueOff();
            } 
            
            if(a_CheckStateBit(g_nReaderState, READER_STAT_PUT_TAG)) 
            {
                g_sReaderLedInfo.time = g_nSysTick;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
            	g_sSoundInfo.state = SOUND_STAT_TX; 
                }
                a_ClearStateBit(g_nReaderState, READER_STAT_PUT_TAG);
                a_SetStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY);
            }
            if( a_CheckStateBit(g_nReaderState, READER_STAT_BIND_TAG)) 
            {
                g_sReaderLedInfo.time = g_nSysTick;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
                	g_sSoundInfo.state = SOUND_STAT_TX; 
                }
                a_ClearStateBit(g_nReaderState, READER_STAT_BIND_TAG);
                a_SetStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY);
            }
            if(a_CheckStateBit(g_nReaderState, READER_STAT_CLASH_TAG)) 
            {
                g_sReaderLedInfo.time = g_nSysTick;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
            	g_sSoundInfo.state = SOUND_STAT_TX; 
                }
                a_ClearStateBit(g_nReaderState, READER_STAT_CLASH_TAG);
                a_SetStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY);
            }

            
            if(a_CheckStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY))
            {
                if(ledTimers & 0x01)
                {
            		Sys_LedRedOn();
                }
                else 
                {
            		Sys_LedRedOff();
                }
                if(g_sReaderLedInfo.time + READER_LED_TIM < g_nSysTick)
                {
					Sys_LedRedOff();
					a_ClearStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY);
                }
            } 
        }
        else
        {
            //灯测
            if(g_sReaderLedInfo.flag == READER_LED_RED_ALL_OFF)
            {
                Sys_LedRedOn();
                g_sReaderLedInfo.time = g_nSysTick;
                g_sReaderLedInfo.flag = READER_LED_RED_ON;
            }
            else if(g_sReaderLedInfo.flag == READER_LED_RED_OFF)
            {
                Sys_LedGreenOn();
                g_sReaderLedInfo.time = g_nSysTick;
                g_sReaderLedInfo.flag = READER_LED_GREEN_ON;
            }
            else if(g_sReaderLedInfo.flag == READER_LED_GREEN_OFF)
            {
                Sys_LedBlueOn();
                g_sReaderLedInfo.time = g_nSysTick;
                g_sReaderLedInfo.flag = READER_LED_BLUE_ON;
            }

            if(g_sReaderLedInfo.flag == READER_LED_RED_ON && g_sReaderLedInfo.time + READER_LEN_LIMIT_TIME < g_nSysTick)
            {
                Sys_LedRedOff();
                g_sReaderLedInfo.flag = READER_LED_RED_OFF;
            }
            else if(g_sReaderLedInfo.flag == READER_LED_GREEN_ON && g_sReaderLedInfo.time + READER_LEN_LIMIT_TIME < g_nSysTick)
            {
                Sys_LedGreenOff();
                g_sReaderLedInfo.flag = READER_LED_GREEN_OFF;
            }
            else if(g_sReaderLedInfo.flag == READER_LED_BLUE_ON && g_sReaderLedInfo.time + READER_LEN_LIMIT_TIME < g_nSysTick)
            {
                Sys_LedBlueOff();
                g_sReaderLedInfo.flag = READER_LED_RED_ALL_OFF;
            }
        }
    #if SYS_ENABLE_WDT
    WDG_FeedIWDog();
    #endif

    }
}

void Sys_KeyTask()
{    
    if(a_CheckStateBit(g_nReaderState, READER_STAT_KEY))
    {
        a_ClearStateBit(g_nReaderState, READER_STAT_KEY);
        Reader_ChkKeyValue(Key_GetValue());
        Key_Control(&g_sKeyValue);
        
        if(g_sKeyValue.flag == KEY_FLAG_FRESH)
        {
            g_sKeyValue.flag = KEY_FLAG_NULL;
            Reader_Init();
        }
    }

}

void Sys_SoundTask()
{
    //串口错误处理:重新初始化
    if(USART_GetFlagStatus(SOUND_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE ))
    {
        USART_ClearFlag(SOUND_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE );
        Sound_InitInterface(SOUND_BAUDRARE);
        Sys_Delayms(10);
        Sound_ConfigInt();
        memset(&g_sSoundInfo, 0, sizeof(SOUND_INFO));
        g_sSoundInfo.state = SOUND_STAT_IDLE; 
    }
  
    if(Sound_IsRcvFrame(g_sSoundInfo))
    {
        a_ClearStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT);
        if(Sounde_Chk(g_sSoundInfo.rxBuf.buffer))
        {
            Sound_ResetFrame(g_sSoundInfo);
        }
    }
   
    if(a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_TX)) 
    {
        a_ClearStateBit(g_sSoundInfo.state, SOUND_STAT_TX);
        g_sSoundInfo.tick = g_nSysTick;
        Sound_TransmitCmd(g_sSoundInfo.txBuf.cmd,0, g_sSoundInfo.txBuf.data);             //测试关闭
        a_SetStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT);
    }
    if(a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT)) 
    {  
        if(g_sSoundInfo.tick + SOUND_VOC_STOP_TIM < g_nSysTick)              
        {
            g_sSoundInfo.repeat ++ ;
            if(g_sSoundInfo.repeat == 3)       
            {
                g_sSoundInfo.repeat = 0;
                a_SetStateBit(g_nSysState, SYS_STAT_SOUND_FAIL);
            }
            a_ClearStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT);
            a_SetStateBit(g_sSoundInfo.state, SOUND_STAT_TX);
        }
    }
    
    if(a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_RCV))
    {
        g_sSoundInfo.repeat = 0;
    }
  
}

    
void Sys_GPBTask()
{
    
    if(USART_GetFlagStatus(GPB_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
    {
        GPB_Stop();
        USART_ClearFlag(GPB_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE); 
        Sys_Delayms(10);
        GPB_Init();
    }
    
    if(Gpb_IsRcvFrame(g_sGpbInfo))
    {
        Gpb_ResetFrame(g_sGpbInfo);
		
		if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
		{
			if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_WIGHT)
			{
				Uart_WriteBuffer(g_sGpbInfo.rxBuf.buffer, g_sGpbInfo.rxBuf.len);
			}
		}
		else
		{
			if(g_sGpbInfo.rxBuf.len >= GPB_BUF_MIN_LEN)
			{
				if(GPB_CheckFrame(&g_sGpbInfo.rxBuf))
				{ 
					a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL); 
					Reader_ResolveWitgh();
				}
			}
			else if(g_sGpbInfo.rxBuf.len == GPB_BUF_MIN_LEN - 1)
			{
				a_ClearStateBit(g_nReaderState, READER_STAT_WIGHT_ZERO_LOADING);
				a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL); 
				a_ClearStateBit(g_sGpbInfo.mode, GPB_WORK_SET_ZERO);
			}
		}

		
    }
    
    if(!a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
    {
        g_sGpbInfo.wightValue = g_sGpbInfo.wightTemp;
    }

    if(g_sGpbInfo.state == GPB_STAT_TX) 
    {
        g_sGpbInfo.tick = g_nSysTick;
        if(g_sGpbInfo.mode == GPB_WOEK_NORMAL)
        {
			GPB_TransmitCmd(&g_sGpbInfo.txBuf);
        }
        else 
		{
			GPB_Adjust(&g_sGpbInfo.txBuf);
			a_SetStateBit(g_nReaderState, READER_STAT_WIGHT_ZERO_LOADING);
		}
		g_sGpbInfo.state = GPB_STAT_WAIT;

    }
    
    if(g_sGpbInfo.state == GPB_STAT_WAIT) 
    {  
        if(g_sGpbInfo.tick + REDAER_UP_GPB_TICK < g_nSysTick) 
        {
            g_sGpbInfo.repeat ++ ;
            g_sGpbInfo.tick = g_nSysTick;
            if(g_sGpbInfo.repeat > REDAER_UP_GPB_NUM)       
            {   
                g_sGpbInfo.repeat = 0;
                a_SetStateBit(g_nSysState, SYS_STAT_GPB_FAIL);   //与称重模块丢失连接
            }  
            g_sGpbInfo.state = GPB_STAT_TX;
        }
    }

     if(a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL))
     {
        if(g_sGpbInfo.flag != GPB_FLAG_FAIL)
        {
            g_sGpbInfo.flag = GPB_FLAG_FAIL;
        }
     }
    else
    {
        if(g_sGpbInfo.flag != GPB_FLAG_NORMAL)
        {
            g_sGpbInfo.flag = GPB_FLAG_NORMAL;
        }
    }
}

void Sys_RfidTask()
{
    if(USART_GetFlagStatus(RFID_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
    {
        USART_ClearFlag(RFID_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        
        USART_ClearFlag(RFID_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE );
        Rfid_InitInterface(RFID_BAUDRARE);
        Sys_Delayms(10);
        Rfid_ConfigInt();
      
    }
    //串口数据帧解析
    if(Uart_IsRcvFrame(g_sRfidInfo.rfidRev))
    {
		memcpy(&g_sRfidRcvTempFrame, &g_sRfidInfo.rfidRev, sizeof(UART_RCVFRAME));
        Uart_ResetFrame(&g_sRfidInfo.rfidRev);
		if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
		{
				if(g_sRfidRcvTempFrame.index >= (UART_FRAME_MIN_LEN - 8))
				{
					if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
					{
						if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_RFID)
						{	
							Uart_WriteBuffer(g_sRfidRcvTempFrame.buffer, g_sRfidRcvTempFrame.index);
						}
					}
				}
		}
		else
		{
			u16 crc1 = 0, crc2 = 0;
			crc1 = Uart_GetFrameCrc(g_sRfidRcvTempFrame.buffer, g_sRfidRcvTempFrame.length);
			crc2 = a_GetCrc(g_sRfidRcvTempFrame.buffer + UART_FRAME_POS_LEN, g_sRfidRcvTempFrame.length - 4);
			if(crc1 == crc2)
			{
				if( *(g_sRfidRcvTempFrame.buffer  + UART_FRAME_POS_CMD + 1) == RFID_GET_UID)
				{
					Reader_RfidGetValue(g_sRfidRcvTempFrame.buffer, &g_sReaderRfidTempInfo);
					if(g_sRfidRcvTempFrame.buffer[READER_RFID_UID_POST])
					{
						a_SetStateBit(g_nReaderState, READER_STAT_RFID_KEEP);
					}
					else
					{
						a_ClearStateBit(g_nReaderState, READER_STAT_RFID_KEEP);
					}
				}
				Rfid_StartOpDelay(g_nSysTick);
				g_nRfidRomNum = a_lfsr(g_nRfidRomNum);
				g_sRfidInfo.delayTick = RFID_OP_DLY_TIM + (g_nRfidRomNum % 10);
				a_ClearStateBit(g_nReaderState, READER_STAT_RFID_FAIL);
			}
			 //g_sRfidInfo.state = RFID_STAT_TX;//测试
		}

    }
    
    if((a_CheckStateBit(g_sRfidInfo.state, RFID_STAT_TX)))//
    {   
        if(g_sLcmInfo.flag !=  LCM_FLAG_PAGE_NULL_CHG)    //初始页面不读卡
        {
            g_sRfidInfo.state = RFID_STAT_WAIT;
            Rfid_TransmitCmd(g_nSysTick);   
        }
    }
    else if(g_sRfidInfo.state == RFID_STAT_DELAY_TIME)
    {
        if(g_sRfidInfo.tick + g_sRfidInfo.delayTick< g_nSysTick)//g_sRfidInfo.delayTick
        {
            g_sRfidInfo.state = RFID_STAT_TX;
        }
    }
 
    if((a_CheckStateBit(g_sRfidInfo.state, RFID_STAT_WAIT)))//
    {   
        if(g_sRfidInfo.tick + RFID_QEQUST_TIM < g_nSysTick)
        {
            g_sRfidInfo.tick = g_nSysTick;
            g_sRfidInfo.repat ++;
            a_SetStateBit(g_sRfidInfo.state, RFID_STAT_TX);
            if(g_sRfidInfo.repat > RFID_QEQUST_TICK)
            {
                g_sRfidInfo.repat = 0;
                a_SetStateBit(g_nReaderState, READER_STAT_RFID_FAIL);
            }
        }
    }


}


void Sys_UartTask(void)
{
    //串口错误处理:重新初始化
    if(USART_GetFlagStatus(UART_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
    {
        USART_ClearFlag(UART_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        
        Uart_EnableInt(DISABLE, DISABLE); 
        Sys_Delayms(10);
        memset(&g_sUartRcvFrame, 0, sizeof(UART_RCVFRAME));
        g_sUartRcvFrame.state = UART_STAT_IDLE; 
        Uart_InitInterface(UART_BAUDRARE);
        Uart_ConfigInt();
        Uart_EnableInt(ENABLE, DISABLE);
    }

    //串口数据帧解析
    {
        if(Uart_IsRcvFrame(g_sUartRcvFrame))
        {
			u16 crc1 = 0, crc2 = 0;
			u16 txLen = 0;
			
            memset(&g_sUartRcvTempFrame, 0, sizeof(UART_RCVFRAME));
            memcpy(&g_sUartRcvTempFrame, &g_sUartRcvFrame, sizeof(UART_RCVFRAME));
            memset(&g_sUartRcvFrame, 0, sizeof(UART_RCVFRAME));
            Lcm_ChkLink(g_sLcmInfo);  
			if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
			{
				if(g_sUartRcvTempFrame.index >= (UART_FRAME_MIN_LEN - 8))
				{
					if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
					{
						if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_RFID)
						{
							Rfid_WriteBuffer(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.index - 1);
						}
						else if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_LCM)
						{
							Lcm_DishWriteBuffer(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.index);
						}
						else if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_WIGHT)
						{
							GPB_WriteBuffer(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.index);
						}
						
						if(g_sUartRcvTempFrame.buffer[UART_FRAME_POS_CMD] == READER_CMD_DRU_CMD)
						{    
							Reader_ProcessUartFrame(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
							if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_RESET)
							{
								txLen = Reader_ProcessUartFrame(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
								if(txLen)
								{
									Uart_WriteBuffer(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
									Sys_Delayms(5);
									Sys_SoftReset();
								}
							}
							
						}
						
					}
				}
			
			}
			else
			{
				if(g_sUartRcvTempFrame.length >= (UART_FRAME_MIN_LEN))
            	{
					
					crc1 = Uart_GetFrameCrc(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
					crc2 = a_GetCrc(g_sUartRcvTempFrame.buffer + UART_FRAME_POS_LEN, g_sUartRcvTempFrame.length - 4);
					if(crc1 == crc2)
					{
						txLen = Reader_ProcessUartFrame(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
						if(txLen && g_sUartRcvTempFrame.buffer[UART_FRAME_POS_PAR - 1] != READER_CMD_GET_MEAL_INFO && g_sUartRcvTempFrame.buffer[UART_FRAME_POS_PAR - 1] != READER_CMD_GET_PERSON_INFO && g_sUartRcvTempFrame.buffer[UART_FRAME_POS_PAR - 2] !=  READER_CMD_CHG_IP )
						{
							a_SetStateBit(g_nReaderState, READER_STAT_UARTTX);
						}
					
					}
				}
			
			}
            Uart_ResetFrame(&g_sUartRcvTempFrame);
        }
        
        if((a_CheckStateBit(g_nReaderState, READER_STAT_UARTTX)))
        {   
            a_ClearStateBit(g_nReaderState, READER_STAT_UARTTX);
            Uart_WriteBuffer(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
            g_sDeviceRspFrame.tick = g_nSysTick;
            a_SetStateBit(g_nReaderState, READER_STAT_UART_TX_WAIT);
            if(g_sDeviceRspFrame.cmd == READER_CMD_RESET)
            {
                Sys_Delayms(5);
                Sys_SoftReset();
            }
            if(g_sDeviceRspFrame.cmd == READER_CMD_DRU_CMD)
            {
                a_SetStateBit(g_nReaderState, READER_STAT_DTU);    
            }
        }
        
        if((a_CheckStateBit(g_nReaderState, READER_STAT_UART_TX_WAIT)))
        {   
            if(g_sDeviceRspFrame.tick + REDAER_UART_TX_TICK < g_nSysTick) 
            {
                if(g_sDeviceRspFrame.status == READER_STATUS_MASTER)
                {
					if(g_sDeviceRspFrame.repeat < REDAER_UP_DATA_NUM)
					{
						g_sDeviceRspFrame.tick = g_nSysTick;
						a_SetStateBit(g_nReaderState, READER_STAT_UARTTX); 
						g_sDeviceRspFrame.repeat ++ ;
					}
					else
					{    
						g_sDeviceRspFrame.status = 0;
						g_sDeviceRspFrame.repeat = 0;
						g_sDeviceRspFrame.tick = g_nSysTick;
						g_sDeviceRspFrame.succFlag = READER_RESFRAME_PERSON_TOTIME;
					}
                }
                else
                { 
					g_sDeviceRspFrame.repeat = 0;
					g_sDeviceRspFrame.tick = g_nSysTick;
                }

                a_ClearStateBit(g_nReaderState, READER_STAT_UART_TX_WAIT);
            }
        }
        
        if((a_CheckStateBit(g_nReaderState, READER_STAT_UARTRX)))
        {   
            a_ClearStateBit(g_nReaderState, READER_STAT_UARTRX);
            Uart_WriteBuffer(g_sDeviceMealRspFrame.buffer, g_sDeviceMealRspFrame.len);
            a_SetStateBit(g_nReaderState, READER_STAT_UART_WAIT);
        }
        
        if((a_CheckStateBit(g_nReaderState, READER_STAT_UART_WAIT)))
        {   
            if(g_sDeviceMealRspFrame.tick + REDAER_UP_DATA_TICK < g_nSysTick) 
            {
                
                if(g_sDeviceMealRspFrame.repeat < REDAER_UP_DATA_NUM)
                {
					g_sDeviceMealRspFrame.repeat ++ ;
					g_sDeviceMealRspFrame.tick = g_nSysTick;
					a_SetStateBit(g_nReaderState, READER_STAT_UARTRX);  
                }
                else     
                {   
					g_sDeviceMealRspFrame.repeat = 0;
					memcpy(FlashTempData, g_sDeviceMealRspFrame.buffer, g_sDeviceMealRspFrame.len);
					g_sDeviceMealRspFrame.tag = REDAER_UPMEALINFO_FAIL;
                }  
                a_ClearStateBit(g_nReaderState, READER_STAT_UART_WAIT);
            }
        }
    }
}

void Sys_LcmTask()
{

 
    if(USART_GetFlagStatus(LCM_DISH_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
    {
        Lcm_Stop();
        Sys_Delayms(5);
        USART_ClearFlag(LCM_DISH_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE); 
		Lcm_Init();
    }

    if(Lcm_IsRcvFrame(g_sLcmDishRcvFrame))
    {
		memcpy(&g_sLcmDishRcvTempFrame, &g_sLcmDishRcvFrame, sizeof(LCM_DISH_RCVFRAME));
		Lcm_EnableRxDma();
        Lcm_ResetFrame(&g_sLcmDishRcvFrame);
		if(g_sLcmDishRcvTempFrame.length)
		{
			if(1)
			{
				 if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
                {
					if(g_sDeviceRspFrame.deviceFlag == READER_CMD_DRU_LCM)
					{
						Uart_WriteBuffer(g_sLcmDishRcvTempFrame.buffer, g_sLcmDishRcvTempFrame.length);
					}
                }
			}
		}
    }

    if(a_CheckStateBit(g_sLcmDishRcvFrame.state, LCM_STAT_TX))
    {
        a_ClearStateBit(g_sLcmDishRcvFrame.state, LCM_STAT_TX);  
        g_sLcmDishTxFrame.tick = g_nSysTick;
        Lcm_EnableTxDma(g_sLcmDishTxFrame.frame , g_sLcmDishTxFrame.len);
        a_SetStateBit(g_sLcmDishRcvFrame.state, LCM_STAT_WAIT);
    }

    if(a_CheckStateBit(g_sLcmDishRcvFrame.state, LCM_STAT_WAIT))
    {
    	if(g_sLcmDishTxFrame.tick + LCM_UP_TICK < g_nSysTick) 
        {
            g_sLcmDishTxFrame.repat ++ ;
            if(g_sLcmDishTxFrame.repat == LCM_UP_NUM)         
            {
                a_SetStateBit(g_nSysState, SYS_STAT_LCM_FAIL);     //显示模块丢失连接
            }
            a_ClearStateBit(g_sLcmDishRcvFrame.state, LCM_STAT_WAIT);  
            a_SetStateBit(g_sLcmDishRcvFrame.state, LCM_STAT_TX);  
        }
    }

}


void Sys_TestTask()
{
    static u8 sampleTick = 0;
    if(a_CheckStateBit(g_nSysState, SYS_STAT_AUTO_TIME))
    {
        a_ClearStateBit(g_nSysState, SYS_STAT_AUTO_TIME); 
        
        if(!a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
        {
            sampleTick++;

            if((sampleTick & 0x01) == 0x00)
            {
               sampleTick = 0;
                a_SetStateBit(g_nReaderState, READER_STAT_LCM_FRESH);
				a_SetStateBit(g_nSysState, SYS_STAT_TEST);
                if(g_sGpbInfo.state != GPB_STAT_WAIT)
                {
                  g_sGpbInfo.state = GPB_STAT_TX;
                }
            }
            a_SetStateBit(g_nReaderState, READER_STAT_KEY | READER_STAT_CHK_LINK | READER_STAT_CHK_UPDATA | READER_STAT_MODE_NORMAL);
        }
    }

    
     g_nRtcTime = RTC_GetCounter();
}


void Sys_ReaderTask()
{ 
    if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
    {
        return ;
    }
    static u16 id = SOUND_VOC_PUT_TAG;
    static u32 offLink = 0;
    Reader_ChgPage(&g_sLcmInfo);
    if(!a_CheckStateBit(g_sDeviceParamenter.reWorkMode, READER_MODE_TEST))
    {
        Reader_NormalMode();
        if(a_CheckStateBit(g_nReaderState, READER_STAT_CHK_LINK))
        {
            a_ClearStateBit(g_nReaderState, READER_STAT_CHK_LINK); 
			
            if(g_nTickLink + READER_LINK_TIME * READER_OFF_LINE_TICK < g_nSysTick)
            {
                g_nTempLink =  READER_LINK_FAIL; 
            }
            if(g_nTempLink == READER_LINK_OK)            	    //未连接是否需要报警
            {
                offLink = 0;
                memset(g_nBufTxt1,0, LCM_INFO_TXT_SIZE);
                Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N + 1, LCM_ICO_ADDR_OFFLINE_BACK);
                a_ClearStateBit(g_nSysState, SYS_STAT_LINE_OFF);
            }
            else if(g_nTempLink ==  READER_LINK_FAIL)
            {
                if(g_nTickLink + READER_LINK_TIME * (READER_OFF_LINE_TICK + offLink) < g_nSysTick)
                {               
					memset(g_nBufTxt1,0, LCM_INFO_TXT_SIZE);
					memcpy(g_nBufTxt1, "断开连接", 8);
					Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_OFFLINE_BACK);
					a_SetStateBit(g_nSysState, SYS_STAT_LINE_OFF);
					g_sSoundInfo.state = SOUND_STAT_TX; 
					g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
					g_sSoundInfo.txBuf.data = SOUND_VOC_OFF_LINE;
					offLink ++; 
                }
            }
        }
    }
    else
    {
		/*
        if(a_CheckStateBit(g_nReaderState, READER_STAT_CHK_LINK))
        {
            a_ClearStateBit(g_nReaderState, READER_STAT_CHK_LINK); 
            if(g_nTempLink ==  READER_LINK_OK)
            {
                if(g_nTickLink + READER_LINK_TIME < g_nSysTick)
                {
            		g_nTempLink =  READER_LINK_FAIL; 
                }
            }
            else if(g_nTempLink ==  READER_LINK_FAIL)
            {
                a_SetStateBit(g_nSysState, SYS_STAT_LINE_OFF);
            }
        }
*/
		
        if(a_CheckStateBit(g_nReaderState, READER_STAT_READER_MARK_OK))     //清除读卡在位标志
        {
            a_ClearStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
        }
		
        if(a_CheckStateBit(g_nSysState, SYS_STAT_TEST))
        { 
            a_ClearStateBit(g_nSysState, SYS_STAT_TEST);  

            Reader_DisplayTest(&g_sRaderInfo.dishInfo , &g_sWightTempInfo);
            Reader_ChgStat(a_CheckStateBit(g_nReaderState, READER_STAT_RFID_FAIL));
            if(g_sSoundInfo.testFlag == SOUND_TEST_FLAG_ENABLE)
            {   
                g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                g_sSoundInfo.txBuf.data = id;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
					g_sSoundInfo.state = SOUND_STAT_TX; 
					id++;
					g_sSoundInfo.testFlag = SOUND_TEST_FLAG_DISABLE; 
					if(id == SOUND_VOC_OFF_LINE + 1)
					{   
						id = SOUND_VOC_PUT_TAG;
					}
                } 
            }

        }

    }
    
    if(g_nOffLineLimitTime + DEVICE_OFFDATA_LIMIT_TIME < g_nSysTick )
    {
        g_nOffLineLimitTime = g_nSysTick;
        if(Reader_SeekNum(g_sReaderOffLineInfo.readerOffLineData))
        {
            memset(&g_sReaderOffLineInfo, 0, sizeof(READER_OFFLINE_INFO));
            Reader_WriteOffLineDataNum();
        }
    }
}
