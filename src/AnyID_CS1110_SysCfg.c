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


const PORT_INF SYS_LED_RED   = {GPIOC, GPIO_Pin_2};
const PORT_INF SYS_LED_BLUE  = {GPIOC, GPIO_Pin_1};
const PORT_INF SYS_LED_GREEN = {GPIOC, GPIO_Pin_0};

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
#if SYS_ENABLE_WDT
    WDG_InitIWDG();
#endif
    
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

   // Reader_WriteOffLineDataNum();
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

    }
}

void Sys_KeyTask()
{
    u8 static k = 0, keyValue = 0;
    
    if(a_CheckStateBit(g_nReaderState, READER_STAT_KEY))
    {
        a_ClearStateBit(g_nReaderState, READER_STAT_KEY);
        if(g_sKeyValue.state == KEY_STAT_IDLE)
        {
            if(Key_GetValue())
            {
                k = Key_GetValue();
                
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
                if(k == Key_GetValue())
                {
                     k = Key_GetValue();
                }
                else
                {
                    g_sKeyValue.value = 0;
                }
                g_sKeyValue.state =KEY_STAT_OK;
            }
            else
            {
                g_sKeyValue.value = Key_GetValue();
                g_sKeyValue.state =KEY_STAT_OK;
            }
        }
        else if(g_sKeyValue.state == KEY_STAT_OK)
        {
            if(g_sKeyValue.tick + KEY_LONG_PRESS_TIME < g_nSysTick)
            {
                if(k == Key_GetValue())
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

                g_sKeyValue.value = Key_GetValue();
                g_sKeyValue.state =KEY_STAT_SAMPLE; 	  

            }
        }
        
        
        if(keyValue != Key_GetValue())
        {
            keyValue = Key_GetValue();
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
            keyValue = Key_GetValue();
        }
    }
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    
    Key_Control(&g_sKeyValue);

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
        Sound_ResetFrame(g_sSoundInfo);
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
	Sys_Delayms(5);
	memset(&(g_sGpbInfo.rxBuf), 0, sizeof(GPB_RX_BUF));
	g_sGpbInfo.state = GPB_STAT_IDLE;
	GPB_InitInterface(GPB_BAUDRARE);
	GPB_ConfigInt(ENABLE);
	GPB_InitTxDma(g_sGpbInfo.txBuf.buffer, g_sGpbInfo.txBuf.len);
	GPB_InitRxDma( g_sGpbInfo.rxBuf.buffer, g_sGpbInfo.rxBuf.len);
	GPB_EnableRxDma();
	
    }
	
    
    if(Gpb_IsRcvFrame(g_sGpbInfo))
    {
        Gpb_ResetFrame(g_sGpbInfo);
        GPB_EnableRxDma();
        a_ClearStateBit(g_sGpbInfo.state, GPB_STAT_WAIT);
        if(g_sGpbInfo.rxBuf.len >= GPB_BUF_MIN_LEN)
        {
            if(GPB_CheckFrame(&g_sGpbInfo.rxBuf))
            { 
                g_sGpbInfo.witghSmple = Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & 0x000FFFFF;
                g_sGpbInfo.repeat = 0;
                g_sWightTempInfo.flag = Gpb_GetValue(g_sGpbInfo.rxBuf.buffer) & GPB_WITGH_MASK_VALUE;
                a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL); 
            }
        }
        else if(g_sGpbInfo.rxBuf.len == GPB_BUF_MIN_LEN - 1)
        {
         
            a_ClearStateBit(g_nReaderState, READER_STAT_WIGHT_ZERO_LOADING);
            a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL); 
            a_ClearStateBit(g_sGpbInfo.mode, GPB_WORK_SET_ZERO);
        }

    }
    
    if(g_nSysState & 0x02)
    {    
        Witgh_CalAvg(&g_sWightTempInfo, g_sGpbInfo.witghSmple);
        g_sGpbInfo.wightTemp = g_sWightTempInfo.avg;
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
    
    if(a_CheckStateBit(g_sGpbInfo.state, GPB_STAT_RCV))
    {
        g_sGpbInfo.repeat = 0; 
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
        g_sRfidInfo.state = RFID_STAT_IDLE;
        if(g_sRfidRcvTempFrame.length >= UART_FRAME_MIN_LEN)
        {
            u16 crc1 = 0, crc2 = 0;
            crc1 = Uart_GetFrameCrc(g_sRfidRcvTempFrame.buffer, g_sRfidRcvTempFrame.length);
            crc2 = a_GetCrc(g_sRfidRcvTempFrame.buffer + UART_FRAME_POS_LEN, g_sRfidRcvTempFrame.length - 4);
            if(crc1 == crc2)
            {
                g_sRfidInfo.repat = 0;
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
               a_ClearStateBit(g_nReaderState, READER_STAT_RFID_FAIL);
            }
        }
        Uart_ResetFrame(&g_sRfidRcvTempFrame);
    }
    
    if((a_CheckStateBit(g_sRfidInfo.state, RFID_STAT_TX)))//
    {   
        g_sRfidInfo.state = RFID_STAT_WAIT;
        Rfid_TransmitCmd(g_nSysTick);   
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
    if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
    {
        if(Uart_IsRcvFrame(g_sUartRcvFrame))
        {
            memcpy(&g_sUartRcvTempFrame, &g_sUartRcvFrame, sizeof(UART_RCVFRAME));
            Uart_ResetFrame(&g_sUartRcvFrame); 
            if(g_sUartRcvTempFrame.length >= UART_FRAME_MIN_LEN)
            {
                u16 crc1 = 0, crc2 = 0;
                crc1 = Uart_GetFrameCrc(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
                crc2 = a_GetCrc(g_sUartRcvTempFrame.buffer + UART_FRAME_POS_LEN, g_sUartRcvTempFrame.length - 4);
                if(crc1 == crc2)
                {
                    if(Reader_DtuUartFrame(g_sUartRcvTempFrame.buffer, &g_sReaderDtuInfo, g_sUartRcvTempFrame.length))
                    {
                    
                    }
                }
            }
            Uart_ResetFrame(&g_sUartRcvTempFrame);
        }
        
        if((a_CheckStateBit(g_nReaderState, READER_STAT_UARTTX)))
        {   
            a_ClearStateBit(g_nReaderState, READER_STAT_UARTTX);
            Uart_WriteBuffer(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
            if(g_sDeviceRspFrame.cmd == READER_CMD_RESET)
            {
                Sys_Delayms(5);
                Sys_SoftReset();
            }
        }
        
    
    
    }
    else
    {
        if(Uart_IsRcvFrame(g_sUartRcvFrame))
        {
            memcpy(&g_sUartRcvTempFrame, &g_sUartRcvFrame, sizeof(UART_RCVFRAME));
            Uart_ResetFrame(&g_sUartRcvFrame);
            Lcm_ChkLink(g_sLcmInfo);   
            if(g_sUartRcvTempFrame.length >= UART_FRAME_MIN_LEN)
            {
                u16 crc1 = 0, crc2 = 0;
                crc1 = Uart_GetFrameCrc(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
                crc2 = a_GetCrc(g_sUartRcvTempFrame.buffer + UART_FRAME_POS_LEN, g_sUartRcvTempFrame.length - 4);
                if(crc1 == crc2)
                {
                    u16 txLen = 0;
                    txLen = Reader_ProcessUartFrame(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length);
                    if(txLen)
                    {
                        a_SetStateBit(g_nReaderState, READER_STAT_UARTTX);
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
        }
        
        if((a_CheckStateBit(g_nReaderState, READER_STAT_UART_TX_WAIT)))
        {   
            if(g_sDeviceRspFrame.tick + REDAER_UART_TX_TICK < g_nSysTick) 
            {
                if(g_sDeviceRspFrame.status == READER_STATUS_MASTER)
                {
                    g_sDeviceRspFrame.repeat ++ ;
                    if(g_sDeviceRspFrame.repeat < REDAER_UP_DATA_NUM)
                    {
                        g_sDeviceRspFrame.tick = g_nSysTick;
                        a_SetStateBit(g_nReaderState, READER_STAT_UARTTX);  
                    }
                    else
                    {    
                        g_sDeviceRspFrame.repeat = 0;
                        g_sDeviceRspFrame.tick = g_nSysTick;
                        g_sDeviceRspFrame.tick = 0;                 //上报数据失败
                        g_sDeviceRspFrame.succFlag = READER_RESFRAME_PERSON_TOTIME;
                       
                    }
                }
                else
                { 
                    g_sDeviceRspFrame.repeat = 0;
                    g_sDeviceRspFrame.tick = g_nSysTick;
                    g_sDeviceRspFrame.tick = 0;                 //上报数据失败
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
                g_sDeviceMealRspFrame.repeat ++ ;
                if(g_sDeviceMealRspFrame.repeat < REDAER_UP_DATA_NUM)
                {
                    g_sDeviceMealRspFrame.tick = g_nSysTick;
                    a_SetStateBit(g_nReaderState, READER_STAT_UARTRX);  
                }
                else if(g_sDeviceMealRspFrame.repeat == REDAER_UP_DATA_NUM)       
                {   
                    g_sDeviceMealRspFrame.repeat = 0;
                    memcpy(FlashTempData, g_sDeviceMealRspFrame.buffer, g_sDeviceMealRspFrame.len);
                    g_sDeviceMealRspFrame.tag = READER_RESFRAME_PERSON_TOTIME;
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
        
        memset(&g_sLcmDishRcvFrame, 0, sizeof(LCM_DISH_RCVFRAME));
        g_sLcmDishRcvFrame.state = LCM_STAT_IDLE;        
        Lcm_DishInitInterface(LCM_DISH_BAUDRARE);
        Lcm_DishConfigInt(ENABLE);
        Lcm_InitTxDma(g_sLcmDishTxFrame.frame, LCM_BUFFER_MAX_LEN);
        Lcm_InitRxDma(g_sLcmDishRcvFrame.buffer, LCM_BUFFER_MAX_LEN);
        Lcm_EnableRxDma();
    }

    if(Lcm_IsRcvFrame(g_sLcmDishRcvFrame))
    {
        Lcm_ResetFrame(&g_sLcmDishRcvFrame);
        Lcm_EnableRxDma();
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
                a_SetStateBit(g_nSysState, SYS_STAT_LCM_FAIL);	 //显示模块丢失连接
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
        sampleTick++;
        if(sampleTick & 0x02)
        {
           sampleTick = 0;
            if(g_sRfidInfo.state != RFID_STAT_WAIT)
            {
                a_SetStateBit(g_sRfidInfo.state, RFID_STAT_TX);
            }
        }
        
        if(g_sGpbInfo.state != GPB_STAT_WAIT)
        {
          g_sGpbInfo.state = GPB_STAT_TX;
        }
        a_SetStateBit(g_nReaderState, READER_STAT_KEY | READER_STAT_LCM_FRESH  | READER_STAT_CHK_LINK | READER_STAT_CHK_UPDATA | READER_STAT_MODE_NORMAL );

    }

    
     g_nRtcTime = RTC_GetCounter();
}


void Sys_ReaderTask()
{ 
    if(a_CheckStateBit(g_nReaderState, READER_STAT_DTU))
    {
        return ;
    }
    static u8 sampleTick = 0, upLink = 0; 
    static u16 id = SOUND_VOC_PUT_TAG;
    static u32 offLink = 0;
    Reader_ChgPage(&g_sLcmInfo);
    if(!a_CheckStateBit(g_sDeviceParamenter.reWorkMode, READER_MODE_TEST))
    {
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
                            if(((g_sGpbInfo.witghSmple - g_sGpbInfo.wightTemp) & 0x80000000))                                                                   
                            {
                                if((g_sGpbInfo.wightTemp - g_sGpbInfo.witghSmple > g_sDeviceParamenter.scoWt))                                                           //未放托盘，但产生取用量
                                {
                                    sampleTick ++;
                                    if(sampleTick >= GPB_STAT_SAMPLE_NUM)
                                    {
                                        sampleTick = 0;
                                        g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                                        g_sSoundInfo.txBuf.data = SOUND_VOC_PUT_TAG;
                                        a_SetStateBit(g_nReaderState, READER_STAT_PUT_TAG);
                                    }
                                }
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
                    memset(&g_sRaderInfo.personInfo, 0, sizeof(PERSON_INFO));
                    if(Gpb_Get_Dish_WightValue() >= 5 && g_nReaderGpbState )
                    {
                        g_sDeviceMealRspFrame.len = Reader_Format_Meal(g_sRaderInfo.dishInfo.type, g_sReaderRfidTempInfo.uid, &g_sDeviceMealRspFrame, g_nRtcTime, Gpb_Get_Dish_WightValue());//RTC???
                        a_SetStateBit(g_nReaderState, READER_STAT_UARTRX);
                        g_sDeviceMealRspFrame.tick = g_nSysTick;
                        g_sDeviceMealRspFrame.mode = READER_RSP_NORMAL_DATA;
                        
                    }
                 }
                
            }
                g_nReaderGpbState = FALSE;

        }
        
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
        if(a_CheckStateBit(g_nReaderState, READER_STAT_CHK_LINK))
        {
            a_ClearStateBit(g_nReaderState, READER_STAT_CHK_LINK); 

            if(g_nTempLink ==  READER_LINK_OK)
            {
                offLink = 0;
                memset(g_nBufTxt1,0, LCM_INFO_TXT_SIZE);
                Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N + 1, LCM_ICO_ADDR_OFFLINE_BACK);
                a_ClearStateBit(g_nSysState, SYS_STAT_LINE_OFF);
                if(g_nTickLink + READER_LINK_TIME < g_nSysTick)
                {
                    g_nTempLink =  READER_LINK_FAIL; 
                }
            }
            else if(g_nTempLink ==  READER_LINK_FAIL)
            {
                if(g_nTickLink + READER_LINK_TIME * (READER_OFF_LINE_TICK + offLink) < g_nSysTick)
                {               
                    memset(g_nBufTxt1,0, LCM_INFO_TXT_SIZE);
                    memcpy(g_nBufTxt1, "断开连接", 8);
                    Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_OFFLINE_BACK);
                    
                    a_SetStateBit(g_nSysState, SYS_STAT_LINE_OFF);
                   // if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                    {
                        g_sSoundInfo.state = SOUND_STAT_TX; 
                        g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                        g_sSoundInfo.txBuf.data = SOUND_VOC_OFF_LINE;
                    }
                    
                    offLink ++; 
                }
                
            }

        }
        
    if(a_CheckStateBit(g_sDeviceMealRspFrame.state, READER_DATA_UP_OK))
    {
        a_ClearStateBit(g_sDeviceMealRspFrame.state, READER_DATA_UP_OK);
        if(Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) > 0)                 //      数据上传成功
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
                     if(Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) > 0)                                //离线数据上传-
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
    
    if(a_CheckStateBit(g_nTempLink, READER_LINK_FAIL))          
    {
        if(Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) > READER_OFFLINE_DATA_BOUNDARY)                                //离线数据超出设定值
        {
            if(g_sReaderOffLineInfo.boundaryNum < Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData))
            {
                g_sReaderOffLineInfo.boundaryNum = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
                g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                g_sSoundInfo.txBuf.data = SOUND_VOC_OFF_LINE;
                a_SetStateBit(g_nSysState, SYS_STAT_OVER_OFFLINE);
            }
            else
            {
                g_sReaderOffLineInfo.boundaryNum = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) ;
            }

        }
    }
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif

    }
    else
    {
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
        if(a_CheckStateBit(g_nReaderState, READER_STAT_READER_MARK_OK))     //清除读卡在位标志
        {
            a_ClearStateBit(g_nReaderState, READER_STAT_READER_MARK_OK);
        }
        if(a_CheckStateBit(g_nSysState, SYS_STAT_TEST))
        { 
            a_ClearStateBit(g_nSysState, SYS_STAT_TEST);  

            Reader_DisplayTest(&g_sRaderInfo.dishInfo , &g_sWightTempInfo);
            Reader_ChgStat(g_nTempLink);
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
    

  

}
