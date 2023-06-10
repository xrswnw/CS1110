#ifndef _ANYID_CS1110_WIGHT_HL_H
#define _ANYID_CS1110_WIGHT_HL_H

#include "AnyID_CS1110_Config.h"

#define GPB_PORT                       USART2
#define GPB_IRQHandler                 USART2_IRQHandler
#define GPB_ChkTxOver()                while(((GPB_PORT)->SR & USART_FLAG_TC) == (u16)RESET)
                                             
#define GPB_BAUDRARE                   115200

#define GPB_ReadByte()                  ((u16)(GPB_PORT->DR & (u16)0x01FF))
extern const PORT_INF GPB_PORT_CTRL;
#define GPB_EnableRx()                     (GPB_PORT_CTRL.Port->BRR  = GPB_PORT_CTRL.Pin)
#define GPB_EnableTx()                     (GPB_PORT_CTRL.Port->BSRR = GPB_PORT_CTRL.Pin)

#define GPB_DMA                               DMA1
#define GPB_TXDMA_CH                          DMA1_Channel7
#define GPB_TXDMA_INT                         DMA1_Channel7_IRQn
#define GPB_TxDMAIRQHandler                   DMA1_Channel7_IRQHandler
#define GPB_TXDMA_TC_FLAG                     DMA1_FLAG_TC7

#define GPB_EnableTxDma(p,s)                  do{\
                                                    (GPB_DMA)->IFCR = GPB_TXDMA_TC_FLAG; \
                                                    (GPB_TXDMA_CH)->CMAR = ((u32)(p)); \
                                                    (GPB_TXDMA_CH)->CNDTR = (s); \
                                                    (GPB_TXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                                }while(0)

#define GPB_DisableTxDma()                    do{\
                                                    (GPB_DMA)->IFCR = GPB_TXDMA_TC_FLAG;\
                                                    (GPB_TXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                                }while(0)

   

#define GPB_RXDMA_CH                          DMA1_Channel6
#define GPB_RXDMA_INT                         DMA1_Channel6_IRQn
#define GPB_RxDMAIRQHandler                   DMA1_Channel6_IRQHandler
#define GPB_RXDMA_TC_FLAG                     DMA1_FLAG_TC6                       

#define GPB_EnableRxDma()                  do{\
												(GPB_DMA)->IFCR = GPB_RXDMA_TC_FLAG; \
                                                (GPB_RXDMA_CH)->CNDTR = GPB_BUFFER_MAX_LEN; \
                                                (GPB_RXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                                }while(0)

#define GPB_DisableRxDma()                    do{\
                                                     (GPB_DMA)->IFCR = GPB_RXDMA_TC_FLAG;\
                                                    (GPB_RXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                                }while(0)
#define GPB_GetRxLen()                     (GPB_BUFFER_MAX_LEN - (GPB_RXDMA_CH)->CNDTR)                                               

void GPB_InitInterface(u32 baudrate);
void GPB_ConfigInt(FunctionalState state);
void GPB_InitTxDma(u8 *pTxBuffer, u32 len);
void GPB_InitRxDma(u8 *pRxBuffer, u32 len);
void GPB_Stop();
void GPB_EnableInt(FunctionalState rxState, FunctionalState txState);
void GPB_WriteByte(u8 ch);
void GPB_Delayms(u32 n);
void GPB_WriteBuffer(u8 *pFrame, u16 len);



#endif