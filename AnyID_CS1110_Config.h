#ifndef _ANYID_CS1110_CONFIG_
#define _ANYID_CS1110_CONFIG_

#include "AnyID_Type.h"
#include "AnyID_Stdlib.h"
#include "stm32f10x_conf.h"

#include <stdio.h>
#include <string.h>


#define SYS_WM_DISH                  0
#define SYS_WM_NORMAL                1

//设定工作时钟
#define SYS_CLOCK_FREQ                  72000000
#define SYS_PROTOCOL_ISO15693           0x00
//#define SYS_PROTOCOL_ISO14443A          0x10
//#define SYS_PROTOCOL_ISO14443B          0x20
#define SYS_PROTOCOL_MASK               0xF0

//STM32平台相关
#if (defined(_ANYID_STM32_) || defined(_ANYID_GD32_))
    #include "stm32f10x.h"

    //端口定义
    typedef struct Port_Inf
    {
        GPIO_TypeDef* Port;
        u16 Pin;
    } PORT_INF;
#endif

#define STM32_CPUID_LEN                 12
#define STM32_CPUID_ADDR                (0x1FFFF7E8)

//优先级分组是2:2
#define INT_PRIORITY_GROUP              2
#define INT_PRIORITY_STICK              0  //STICK的优先级最低
#define INT_PRIORITY_UART_RX            14  //串口接收
#define INT_PRIORITY_UART_TXDMA         8  //串口发送DMA
#define INT_PRIORITY_UART_RXDMA         13                 
#define INT_PRIORITY_PCD_IO             4   //15693的发送定时器优先级最高
#define INT_PRIORITY_LOW                14

#define Sys_EnableInt()     __enable_irq()
#define Sys_DisableInt()    __disable_irq()
#define Sys_SoftReset()     (*((u32 *)0xE000ED0C) = 0x05fa0004)
#endif


