/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"


/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    g_nSysTick ++;
  //心跳时钟调整，设置5ms心跳     

    if((g_nSysTick % 11) == 0)
    {
        a_SetStateBit(g_nSysState, SYS_STAT_TEST | SYS_STAT_RUNLED | SYS_STAT_AUTO_TIME );

    }
    


  
}


void SOUND_IRQHandler(void)
{
    if(SOUND_PORT->SR & USART_FLAG_RXNE)
    {
        u8 byte =0;
        
        USART_ClearFlag(SOUND_PORT,USART_FLAG_RXNE);
        byte =  Sound_ReadByte();
        Sound_ReceiveFrame(byte,&g_sSoundInfo);
    }
    else
    {
        Sound_ReadByte();
    }
    SOUND_PORT->SR &= (~0x3FF);
}


//定义一个全局变量，不需要每次申请局部变量
u16 g_nGpbSr = 0;
u16 g_nGpbDr = 0;
void GPB_IRQHandler(void)
{
    if(GPB_PORT->SR & USART_FLAG_IDLE)
    {    
        GPB_DisableRxDma();
        g_sGpbInfo.rxBuf.len = GPB_GetRxLen(); 
        g_sGpbInfo.state = GPB_STAT_RCV;    //接收数据完成
    }    
    
    g_nGpbSr = GPB_PORT->SR;  //通过读取SR和DR清空中断标志
    g_nGpbDr = GPB_PORT->DR;    
}

void GPB_RxDMAIRQHandler(void)
{
    GPB_DisableRxDma(); //接收缓冲区满，一般不会出现这样的情况，如果有，就可能系统有故障
    g_sGpbInfo.rxBuf.len = GPB_GetRxLen(); 
    g_sGpbInfo.state = GPB_STAT_RCV;    //接收数据完成
}

void GPB_TxDMAIRQHandler(void)
{
    GPB_DisableTxDma();                    //DMA完成后，最后一个字节可能没有发送出去，需要在主循环中做判断处理
    g_sGpbInfo.state = GPB_STAT_TX_IDLE;
}




u16 g_nLcmSr = 0;
u16 g_nLcmDr = 0;
void Lcm_DishIRQHandler(void)
{
    if(LCM_DISH_PORT->SR & USART_FLAG_IDLE)
    {    
        Lcm_DisableRxDma();
        g_sLcmDishRcvFrame.length = Lcm_GetRxLen() ; 
        g_sLcmDishRcvFrame.state = LCM_STAT_OVR;    //接收数据完成
    }    
    
    g_nLcmSr = LCM_DISH_PORT->SR;  //通过读取SR和DR清空中断标志
    g_nLcmDr = LCM_DISH_PORT->DR;    
}

void LCM_RxDMAIRQHandler(void)
{
    Lcm_DisableRxDma(); //接收缓冲区满，一般不会出现这样的情况，如果有，就可能系统有故障
    g_sLcmDishRcvFrame.length = GPB_GetRxLen(); 
    g_sLcmDishRcvFrame.state = LCM_STAT_OVR;    //接收数据完成
}

void LCM_TxDMAIRQHandler(void)
{
    Lcm_DisableTxDma();                    //DMA完成后，最后一个字节可能没有发送出去，需要在主循环中做判断处理
    g_sLcmDishRcvFrame.state = GPB_STAT_TX_IDLE;
}




void Uart_IRQHandler(void)
{
  if(USART_GetITStatus(UART_PORT, USART_IT_RXNE) != RESET)
    {
        u8 byte = 0;
        USART_ClearITPendingBit(UART_PORT, USART_IT_RXNE);
        byte = Uart_ReadByte();
        Uart_ReceiveFrame(byte, &g_sUartRcvFrame);
    }
    else 
    {
        Uart_ReadByte();
    }
    UART_PORT->SR &= (~0x3FF);
}

void RFID_IRQHandler(void)
{
  if(USART_GetITStatus(RFID_PORT, USART_IT_RXNE) != RESET)
    {
        u8 byte = 0;
        USART_ClearITPendingBit(RFID_PORT, USART_IT_RXNE);
        byte = Rfid_ReadByte();
        Rfid_ReceiveFrame(byte, &g_sRfidInfo.rfidRev);
    }
    else 
    {
        Rfid_ReadByte();
    }
    UART_PORT->SR &= (~0x3FF);
}


 
void RTC_IRQHandler(void)
{

    if(RTC_GetITStatus(RTC_IT_OW)  == RESET)
    {
        RTC_ClearFlag(RTC_IT_OW);     //更新RTC，不需要报警
        //Rtc_UpdateAlarmTime(RTC_ALARM_TIME);
        RTC_SetCounter(0);
        g_sRtcTime.counter ++;
        // Wait for RTC registers synchronization
        RTC_WaitForSynchro();
        // Wait until last write operation on RTC registers has finished
        RTC_WaitForLastTask();
    }
}


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
