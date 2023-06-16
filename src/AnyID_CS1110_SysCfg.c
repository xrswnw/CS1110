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
#if SYS_ENABLE_WDT
    WDG_InitIWDG();
#endif
    


    Sys_LedRedOn();
    Sys_LedGreenOn();
    Sys_LedBlueOn();
    FRam_InitInterface();
    Flash_InitInterface();
    Flash_Init();

    Reader_WriteOffLineDataNum();
    Reader_ReadOffLineDataNum();
    Fram_Demo();
    

    Rfid_Init();
    Lcm_Init();
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    
    Sys_Delayms(500);                           //LCD初始化时间，暂定
    
    
#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    
    Reader_Init();
    Sound_Init();
    GPB_Init();

    Uart_Init();
    
    Flash_Demo();
    //SysTick 初始化 5ms

    /*
    Reader_ReadOffLineDatas((0 * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
    Reader_ReadOffLineDatas((1 * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
    Reader_ReadOffLineDatas((2 * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
    */  
    Sys_LedRedOff();
    Sys_LedGreenOff();
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
        
        if(a_CheckStateBit(g_sGpbInfo.mode, GPB_WORK_SET_ZERO))
        {
            a_ClearStateBit(g_nSysState, GPB_WORK_SET_ZERO);
            Sys_LedBlueOn();
        }
        else
        {
            Sys_LedBlueOff();
        }
        
   
        
        if(a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL))
        {
             a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL);
            if(ledTimers & 0x04)
            {
               Sys_LedBlueOn();
            }
            else 
            {
                Sys_LedBlueOff();
            }
        }
        else
        {
            Sys_LedBlueOff();
        }
        
        if(a_CheckStateBit(g_nSysState, SYS_STAT_RFID_KEEP))
        {
            Sys_LedRedOn();
        }
        else
        {
             Sys_LedRedOff();
        }
        
        if(ledTimers & 0x0A)               
        {
            
            if(a_CheckStateBit(g_nSysState, SYS_STAT_PUT_TAG)) 
            {
                g_nSysGLed = g_nSysTick;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
                    g_sSoundInfo.state = SOUND_STAT_TX; 
                }
                 
                a_ClearStateBit(g_nSysState, SYS_STAT_PUT_TAG);//
                a_SetStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY);
            }
            if(a_CheckStateBit(g_nSysState, SYS_STAT_BIND_TAG)) 
            {
                g_nSysBLed = g_nSysTick;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
                    g_sSoundInfo.state = SOUND_STAT_TX; 
                } 
                a_ClearStateBit(g_nSysState, SYS_STAT_BIND_TAG);//
                a_SetStateBit(g_sReaderLedInfo.state, SYS_STAT_B_LED_DELAY);
            }
            
            if(a_CheckStateBit(g_nSysState, SYS_STAT_OVER_OFFLINE)) 
            {
                g_nSysBLed = g_nSysTick;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
                    g_sSoundInfo.state = SOUND_STAT_TX; 
                } 
                a_ClearStateBit(g_nSysState, SYS_STAT_OVER_OFFLINE);//
                a_SetStateBit(g_sReaderLedInfo.state, SYS_STAT_G_LED_DELAY);
            }
            
            if(a_CheckStateBit(g_nSysState, SYS_STAT_LINE_OFF | SYS_STAT_CLASH_TAG)) 
            {
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
                    g_sSoundInfo.state = SOUND_STAT_TX; 
                } 
                a_ClearStateBit(g_nSysState, SYS_STAT_LINE_OFF | SYS_STAT_CLASH_TAG);//
            }

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
            if(g_nSysGLed + READER_LED_TIM < g_nSysTick)
            {
                Sys_LedRedOff();
                 a_ClearStateBit(g_sReaderLedInfo.state, SYS_STAT_R_LED_DELAY);
                
            }
        } 
        if(a_CheckStateBit(g_sReaderLedInfo.state, SYS_STAT_B_LED_DELAY))
        {
            if(ledTimers & 0x01)
            {
                Sys_LedBlueOn();
            }
            else 
            {
                Sys_LedBlueOff();
            }
            if(g_nSysBLed + READER_LED_TIM < g_nSysTick)
            {
                Sys_LedBlueOff();
                a_ClearStateBit(g_sReaderLedInfo.state, SYS_STAT_B_LED_DELAY);
            }
        }
        if(a_CheckStateBit(g_sReaderLedInfo.state, SYS_STAT_G_LED_DELAY))
        {
            if(ledTimers & 0x01)
            {
                Sys_LedBlueOn();
            }
            else 
            {
                Sys_LedBlueOff();
            }
            if(g_nSysBLed + READER_LED_TIM < g_nSysTick)
            {
                Sys_LedBlueOff();
                a_ClearStateBit(g_sReaderLedInfo.state, SYS_STAT_G_LED_DELAY);
            }
        }
    }
}

void Sys_KeyTask()
{
    u8 static k = 0;
    
    if(a_CheckStateBit(g_nSysState, SYS_STAT_KEY))
    {
        a_ClearStateBit(g_nSysState, SYS_STAT_KEY);
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
                g_sKeyValue.value = 0x00;
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
                    g_sKeyValue.value = 0x00;
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
        u8 bok = 0;
        a_ClearStateBit(g_sSoundInfo.state, SOUND_STAT_RCV);
        bok = Sounde_Chk(g_sSoundInfo.rxBuf.buffer);
        if(bok)
        {
            Sound_ResetFrame(g_sSoundInfo);
            a_ClearStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT);
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
        if(g_sSoundInfo.tick + SOUND_VOC_STOP_TIM < g_nSysTick)                //等待语音结束后才有反馈
        {
            g_sSoundInfo.repeat ++ ;
            if(g_sSoundInfo.repeat == 3)       
            {
                g_sSoundInfo.repeat = 0;
                a_SetStateBit(g_nSysState, SYS_STAT_SOUND_FAIL);                //与语音模块丢失连接
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
    static u8 sampleTick = 0;
    static u32 sampleWitgh = 0;
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
        if(g_sGpbInfo.rxBuf.len >= GPB_BUF_MIN_LEN)
        {
            if(GPB_CheckFrame(&g_sGpbInfo.rxBuf))
            { 
                g_sGpbInfo.wightTemp =  (g_sGpbInfo.rxBuf.buffer[GPB_WIGHT_VALUE_POS] << 24)     |
                                        (g_sGpbInfo.rxBuf.buffer[GPB_WIGHT_VALUE_POS + 1] << 16) |
                                        (g_sGpbInfo.rxBuf.buffer[GPB_WIGHT_VALUE_POS + 2] <<  8) |
                                        (g_sGpbInfo.rxBuf.buffer[GPB_WIGHT_VALUE_POS + 3] << 0)  ;
                if(!a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
                {
                    if((g_sGpbInfo.wightTemp - sampleWitgh <= 3) || (sampleWitgh - g_sGpbInfo.wightTemp <= 3))
                    {
                        sampleTick ++;
                        sampleWitgh = g_sGpbInfo.wightTemp;
                        if(sampleTick >= GPB_STAT_SAMPLE_NUM )
                        {
                            sampleTick = 0;
                            g_sGpbInfo.wightValue = Sound_GetValue(sampleWitgh);
                        }
                    }
                    else
                    {
                         sampleWitgh = g_sGpbInfo.wightTemp;
                         sampleTick = 0;
                    }
                }
                a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL); 
            }
        }
        else if(g_sGpbInfo.rxBuf.len == GPB_BUF_MIN_LEN - 1)
        {
            //零点重置完成
            a_ClearStateBit(g_nSysState, SYS_STAT_GPB_FAIL); 
            a_ClearStateBit(g_sGpbInfo.mode, GPB_WORK_SET_ZERO);
        }

    }

    if(a_CheckStateBit(g_sGpbInfo.state, GPB_STAT_TX)) 
    {
        a_ClearStateBit(g_sGpbInfo.state, GPB_STAT_TX);
        g_sGpbInfo.tick = g_nSysTick;

        if(g_sGpbInfo.mode == GPB_WOEK_NORMAL)
        {
          //GPB_Filt_Chg();
            GPB_TransmitCmd(&g_sGpbInfo.txBuf);
        }
        else 
        {
            GPB_Adjust(&g_sGpbInfo.txBuf);
        }
        a_SetStateBit(g_sGpbInfo.state, GPB_STAT_WAIT);
        
    }
    if(a_CheckStateBit(g_sGpbInfo.state, GPB_STAT_WAIT)) 
    {  
        if(g_sGpbInfo.tick + REDAER_UP_GPB_TICK < g_nSysTick) 
        {
            g_sGpbInfo.repeat ++ ;
            if(g_sGpbInfo.repeat == REDAER_UP_GPB_NUM)       
            {   
                g_sGpbInfo.repeat = 0;
                a_SetStateBit(g_nSysState, SYS_STAT_GPB_FAIL);   //与称重模块丢失连接
            }  
            a_ClearStateBit(g_sGpbInfo.state, GPB_STAT_WAIT);
            a_SetStateBit(g_sGpbInfo.state, GPB_STAT_TX);  
        }
    }
    
    if(a_CheckStateBit(g_sGpbInfo.state, GPB_STAT_RCV))
    {
        g_sGpbInfo.repeat = 0; 
    }

}

void Sys_RfidTask()
{
  
/*
    if(a_CheckStateBit(g_nSysState, SYS_STAT_AUTO_RFID))
    {   
        a_ClearStateBit(g_nSysState, SYS_STAT_AUTO_RFID);
        if(Reader_RfidGetValue(READER_RFID_READ_UID, &g_sReaderRfidTempInfo) == ISO15693_ERR_SUC)
        {
            a_SetStateBit(g_nSysState, SYS_STAT_RFID_KEEP);
        }else
        {
            a_ClearStateBit(g_nSysState, SYS_STAT_RFID_KEEP);
        }
    }
    */ 
   

    
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

        if(g_sRfidRcvTempFrame.length >= UART_FRAME_MIN_LEN)
        {
            u16 crc1 = 0, crc2 = 0;
            crc1 = Uart_GetFrameCrc(g_sRfidRcvTempFrame.buffer, g_sRfidRcvTempFrame.length);
            crc2 = a_GetCrc(g_sRfidRcvTempFrame.buffer + UART_FRAME_POS_LEN, g_sRfidRcvTempFrame.length - 4);
            if(crc1 == crc2)
            {
                if( *(g_sRfidRcvTempFrame.buffer  + UART_FRAME_POS_CMD + 1) == RFID_GET_UID)
                {
                    Reader_RfidGetValue(g_sRfidRcvTempFrame.buffer, &g_sReaderRfidTempInfo);
                    if(g_sRfidRcvTempFrame.buffer[19])
                    {
                        a_SetStateBit(g_nSysState, SYS_STAT_RFID_KEEP);
                    }
                    else
                    {
                        a_ClearStateBit(g_nSysState, SYS_STAT_RFID_KEEP);
                    }
                }
            }
        }
        Uart_ResetFrame(&g_sRfidRcvTempFrame);
    }
    
    if((a_CheckStateBit(g_sRfidInfo.state, RFID_STAT_TX)))//
    {   
        a_ClearStateBit(g_sRfidInfo.state, RFID_STAT_TX);
        Rfid_TransmitCmd();   
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
                    a_SetStateBit(g_nSysState, SYS_STAT_UARTTX);
                }
            }
        }
        Uart_ResetFrame(&g_sUartRcvTempFrame);
    }
    
    if((a_CheckStateBit(g_nSysState, SYS_STAT_UARTTX)))//
    {   
        a_ClearStateBit(g_nSysState, SYS_STAT_UARTTX);
        Uart_WriteBuffer(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
        if(g_sDeviceRspFrame.cmd == READER_CMD_RESET)
        {
            Sys_Delayms(5);
            Sys_SoftReset();
        }
    }
    
    if((a_CheckStateBit(g_nSysState, SYS_STAT_UARTRX)))//
    {   
        a_ClearStateBit(g_nSysState, SYS_STAT_UARTRX);
        Uart_WriteBuffer(g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
        a_SetStateBit(g_nSysState, SYS_STAT_UART_WAIT);

    }
    
    if((a_CheckStateBit(g_nSysState, SYS_STAT_UART_WAIT)))//
    {   
        if(g_sDeviceRspFrame.tick + REDAER_UP_DATA_TICK < g_nSysTick) 
        {
            g_sDeviceRspFrame.repeat ++ ;
            if(g_sDeviceRspFrame.repeat < REDAER_UP_DATA_NUM)
            {
                g_sDeviceRspFrame.tick = g_nSysTick;
                a_SetStateBit(g_nSysState, SYS_STAT_UARTRX);  
            }
            else if(g_sDeviceRspFrame.repeat == REDAER_UP_DATA_NUM)       
            {   
                g_sDeviceRspFrame.repeat = 0;
                memcpy(FlashTempData, g_sDeviceRspFrame.buffer, g_sDeviceRspFrame.len);
                a_SetStateBit(g_nSysState, SYS_STAT_OFF_LINE);   //上传取餐信息失败
            }  
            a_ClearStateBit(g_nSysState, SYS_STAT_UART_WAIT);
            
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
  
  /*
  Lcm_SelectPage(1);
Sys_Delayms(5);
    Lcm_SelectPage(2);
    Sys_Delayms(5);
      Lcm_SelectPage(3);
      Sys_Delayms(5);
        Lcm_SelectPage(4);
        Sys_Delayms(5);
*/
    //Reader_Seek_Inq(tempData);
  /*u8 test = 0;//测试页面切换

    if(a_CheckStateBit(g_nSysState, SYS_STAT_LCM_TX))  
    {  
        a_ClearStateBit(g_nSysState, SYS_STAT_LCM_TX);

        Lcm_SelectPage(test + g_sDeviceParamenter.uiMode);
        test++;
        if(test == 1)
        {
            Lcm_BackgroundCtr(0xC2, LCM_BACKGROUND_ADDR_WITGH);
            Lcm_BackgroundCtr(0xC2, LCM_BACKGROUND_ADDR_MONEY);
            Lcm_BackgroundCtr(0xC2, LCM_BACKGROUND_ADDR_TOTAL); 
        }
        else if(test == 4)
        {
            test = 0;
        }
    }*/
  
  

    

}


void Sys_ReaderTask()
{ 
    static u8 sampleTick = 0, upLink = 0, offLink = 0; 
  
    Reader_ChgPage(&g_sLcmInfo);
    if(!a_CheckStateBit(g_sDeviceParamenter.reWorkMode, READER_MODE_TEST))
    {
        if(a_CheckStateBit(g_nSysState, SYS_STAT_LCM_FRESH))
        {   
            a_ClearStateBit(g_nSysState, SYS_STAT_LCM_FRESH); 
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

                g_sDeviceRspFrame.len = Reader_Format_GetPerson(g_sReaderRfidTempInfo.uid, &g_sDeviceRspFrame, g_nSysTick);//RTC???,,暂用g_nSysTick代替
                g_sReaderRfidTempInfo.tempState = RFID_TAG_IN;
                a_SetStateBit(g_nSysState, SYS_STAT_UARTTX);
            }
        }
        if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_OUT) && (!a_CheckStateBit(g_nSysState, SYS_STAT_GPB_FAIL)))
        {
            memset(&g_sRaderInfo.personInfo, 0, sizeof(PERSON_INFO));
            if((Lcm_ChkPage(LCM_PAGE_MAIN_WHITE) || Lcm_ChkPage(LCM_PAGE_INFO_WHITE)) )
            {
                if(Lcm_ChkPage(LCM_PAGE_MAIN_WHITE))
                {
                    if(!a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
                    {
                        if(a_CheckStateBit(g_nSysState, SYS_STAT_MODE_NORMAL))
                        {
                             a_ClearStateBit(g_nSysState, SYS_STAT_MODE_NORMAL) ;
                            if(!(Reader_GetWight(&g_sGpbInfo) & 0x10000000))                                                                    /*g_sGpbInfo.wightValue > Sound_GetValue(g_sGpbInfo.wightTemp)  && */
                            {
                                if(Reader_GetWight(&g_sGpbInfo) > g_sDeviceParamenter.scoWt)                                                           //未放托盘，但产生取用量
                                {
                                    sampleTick ++;
                                    if(sampleTick >= GPB_STAT_SAMPLE_NUM - 1)
                                    {
                                        sampleTick = 0;
                                        g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                                        g_sSoundInfo.txBuf.data = SOUND_VOC_PUT_TAG;
                                        a_SetStateBit(g_nSysState, SYS_STAT_PUT_TAG);
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
                    if(Reader_GetWight(&g_sGpbInfo) >= 5 && g_nReaderGpbState)
                    {
                        g_sDeviceRspFrame.len = Reader_Format_Meal(g_sRaderInfo.dishInfo.type, g_sReaderRfidTempInfo.uid, &g_sDeviceRspFrame, g_nSysTick, Reader_GetWight(&g_sGpbInfo));//RTC???
                        a_SetStateBit(g_nSysState, SYS_STAT_UARTRX);
                        g_sDeviceRspFrame.tick = g_nSysTick;
                        g_sDeviceRspFrame.mode = READER_RSP_NORMAL_DATA;
                        
                    }
                 }
                
            }
                g_nReaderGpbState = FALSE;

        }
        
        if(a_CheckStateBit(g_sReaderRfidTempInfo.state, RFID_TAG_KEEP))
        {
            if(Lcm_ChkPage(LCM_PAGE_MAIN_WHITE))
            {
                //if(g_sRaderInfo.personInfo.state == READER_RESFRAME_PERSON_OK)                        //获取用户信息
                {
                    if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AIR)
                    {

                       Lcm_SetPage(LCM_FLAG_PAGE_INFO_CHG);
                    }
                    else if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AFTER)
                    {
                        Lcm_SetPage(LCM_FLAG_PAGE_INFO_CHG);
                        a_ClearStateBit(g_nSysState, SYS_STAT_PUT_TAG);
                    }
                }
                
            }
            else if(Lcm_ChkPage(LCM_PAGE_INFO_WHITE))
            {
                if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AIR)
                {
                  
                    if(g_sRaderInfo.personInfo.bind == READER_PERSON_BIND_FAIL)
                    {
                        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_WARN);
                        g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                        g_sSoundInfo.txBuf.data = SOUND_VOC_NO_BIND;
                        a_SetStateBit(g_nSysState, SYS_STAT_BIND_TAG);  
                    }
                    else
                    {
                        a_ClearStateBit(g_nSysState, SYS_STAT_PUT_TAG);
                    }
                    if(g_sRaderInfo.personInfo.allergy == READER_PERSON_ALLERGY)
                    {
                        memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
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
                        memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
                        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY);
                        Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY_BACK);
                    }
                }
                else if(g_sDeviceParamenter.totalMode == READER_TOTAL_MODE_AFTER)
                {
                    Lcm_BackgroundCtr(LCM_BACKGROUND_STR_NORMAL, LCM_BACKGROUND_ADDR_INFO);
                    Lcm_BackgroundCtr(LCM_BACKGROUND_STR_NORMAL, LCM_BACKGROUND_ADDR_ALLERGY);
                    memset(g_nBufTxt,0, LCM_INFO_TXT_SIZE);
                    Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY);
                    Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N, LCM_ICO_ADDR_ALLERGY_BACK);
            
                }
            }
        }

        if(a_CheckStateBit(g_sReaderRfidTempInfo.clash, RFID_TAG_FAIL))
        {
            a_ClearStateBit(g_sReaderRfidTempInfo.clash, RFID_TAG_FAIL);
            a_SetStateBit(g_nSysState, SYS_STAT_CLASH_TAG);
            g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
            g_sSoundInfo.txBuf.data = SOUND_VOC_TAG_CLASH;

        }
        if(a_CheckStateBit(g_nSysState, SYS_STAT_CHK_LINK))
        {
            a_ClearStateBit(g_nSysState, SYS_STAT_CHK_LINK); 

            if(g_nTempLink ==  READER_LINK_OK)
            {
                offLink = 0;
                memset(g_nBufTxt1,0, LCM_INFO_TXT_SIZE);
                Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_N + 1, LCM_ICO_ADDR_OFFLINE_BACK);
                if(g_nTickLink + READER_LINK_TIME < g_nSysTick)
                {
                    g_nTempLink =  READER_LINK_FAIL; 
                }
            }
            else if(g_nTempLink ==  READER_LINK_FAIL)
            {
                memset(g_nBufTxt1,0, LCM_INFO_TXT_SIZE);
                memcpy(g_nBufTxt1, "断开连接", 8);
                Lcm_SelectIco(LCM_CTR_ICO_DISPLAY_L, LCM_ICO_ADDR_OFFLINE_BACK);
                
                if(g_nTickLink + READER_LINK_TIME * (READER_OFF_LINE_TICK + offLink) < g_nSysTick)
                {
                    offLink ++;
                    g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                    g_sSoundInfo.txBuf.data = SOUND_VOC_OFF_LINE;
                    a_SetStateBit(g_nSysState, SYS_STAT_LINE_OFF);
                }
                
            }

        }
        
    }
    else
    {
        if(a_CheckStateBit(g_nSysState, SYS_STAT_TEST))
        { 
            a_ClearStateBit(g_nSysState, SYS_STAT_TEST);  
            u16 static id = SOUND_VOC_PUT_TAG;
            
            Reader_DisplayTest(&g_sRaderInfo.dishInfo , &g_sGpbInfo);
            Reader_ChgTag(g_sReaderRfidTempInfo.state);
            
            if(1)
            {   
                g_sSoundInfo.txBuf.cmd = SOUND_FRAME_CMD_APPOINT_NUM;
                g_sSoundInfo.txBuf.data = id;
                if(!a_CheckStateBit(g_sSoundInfo.state, SOUND_STAT_WAIT))
                {
                    g_sSoundInfo.state = SOUND_STAT_TX; 
                    id++;
                    if(id == SOUND_VOC_OFF_LINE + 1)
                    {   
                      id = SOUND_VOC_PUT_TAG;

                    }
                } 
            }

        }
    }
    

    if(a_CheckStateBit(g_sDeviceRspFrame.state, READER_DATA_UP_OK))
    {
        a_ClearStateBit(g_sDeviceRspFrame.state, READER_DATA_UP_OK);
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
        a_ClearStateBit(g_nSysState, SYS_STAT_UART_WAIT);
    }
    
    
    if(a_CheckStateBit(g_nSysState, SYS_STAT_OFF_LINE))
    {
        a_ClearStateBit(g_nSysState, SYS_STAT_OFF_LINE);    //信息上传失败

        if(g_sDeviceRspFrame.mode == READER_RSP_NORMAL_DATA)
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
        if(!a_CheckStateBit(g_nSysState, SYS_STAT_UART_WAIT))
        {       
            //g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
            if(a_CheckStateBit(g_nSysState, SYS_STAT_CHK_UPDATA))
            {
                upLink ++;
                a_ClearStateBit(g_nSysState, SYS_STAT_CHK_UPDATA);    
                if(upLink == REDAER_UP_DATA_TICK)
                {
                    upLink = 0;
                     if(Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData) > 0)                                //离线数据上传-
                    {
                        g_sReaderOffLineInfo.index = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);
                        g_sFramData.id = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);

                        Reader_ReadOffLineDatas((g_sFramData.id * FRAM_OFFLINE_DATA) + FRAM_OFF_LINE_ADDR, FRAM_OFFLINE_DATA_LEN, g_sFramData.pBuffer);
                        memcpy(&g_sDeviceRspFrame.buffer, &g_sFramData.pBuffer, READER_RSP_OFFLINE_DATA_LEN);
                        g_sDeviceRspFrame.len = READER_RSP_OFFLINE_DATA_LEN;
                        a_SetStateBit(g_nSysState, SYS_STAT_UARTRX);
                        g_sDeviceRspFrame.tick = g_nSysTick;
                        g_sDeviceRspFrame.mode = READER_RSP_OFFLINE_DATA;
                    }
                
                }
            }
        }
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