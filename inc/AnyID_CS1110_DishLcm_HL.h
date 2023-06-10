#ifndef _ANYID_CS1110_DISHLCM_HL_H
#define _ANYID_CS1110_DISHLCM_HL_H

#include "AnyID_CS1110_Config.h"

#define LCM_BUFFER_MAX_LEN             (256 + 32) 
                                             
#define LCM_DISH_BAUDRARE                   115200

#define Lcm_DishReadByte()                  ((u16)(LCM_DISH_PORT->DR & (u16)0x01FF))

#define LCM_DISH_PORT                           USART1
#define LCM_DISH_BAUDRARE                       115200
#define Lcm_DishIRQHandler                     USART1_IRQHandler
#define Lcm_ChkTxOver()                    while(((LCM_DISH_PORT)->SR & USART_FLAG_TC) == (u16)RESET)



#define Lcm_ReadByte()                     ((u16)(LCM_DISH_PORT->DR & (u16)0x01FF))


                                                
#define LCM_SR_IDLE                        0x0010  
#define LCM_DMA                            DMA1

#define LCM_TXDMA_CH                       DMA1_Channel4
#define LCM_TXDMA_INT                      DMA1_Channel4_IRQn
#define LCM_TxDMAIRQHandler                DMA1_Channel4_IRQHandler
#define LCM_TXDMA_TC_FLAG                  DMA1_FLAG_TC4
#define Lcm_EnableTxDma(p, s)              do{\
                                                (LCM_DMA)->IFCR = LCM_TXDMA_TC_FLAG; \
                                                (LCM_TXDMA_CH)->CMAR = ((u32)(p)); \
                                                (LCM_TXDMA_CH)->CNDTR = (s); \
                                                (LCM_TXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                             }while(0)
                                                                                             
#define Lcm_DisableTxDma()                 do{\
                                                (LCM_DMA)->IFCR = LCM_TXDMA_TC_FLAG;\
                                                (LCM_TXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                                Lcm_ChkTxOver();\
                                             }while(0)


#define LCM_RXDMA_CH                       DMA1_Channel5
#define LCM_RXDMA_INT                      DMA1_Channel5_IRQn
#define LCM_RxDMAIRQHandler                DMA1_Channel5_IRQHandler
#define LCM_RXDMA_TC_FLAG                  DMA1_FLAG_TC5
#define Lcm_EnableRxDma()                  do{\
                                                (LCM_DMA)->IFCR = LCM_RXDMA_TC_FLAG; \
                                                (LCM_RXDMA_CH)->CNDTR = LCM_BUFFER_MAX_LEN; \
                                                (LCM_RXDMA_CH)->CCR |= CCR_ENABLE_Set; \
                                            }while(0)
                                            
#define Lcm_GetRxLen()                     (LCM_BUFFER_MAX_LEN - (LCM_RXDMA_CH)->CNDTR)  

#define Lcm_DisableRxDma()                 do{\
                                                (LCM_DMA)->IFCR = LCM_RXDMA_TC_FLAG;\
                                                (LCM_RXDMA_CH)->CCR &= CCR_ENABLE_Reset;\
                                            }while(0)
void Lcm_Stop(void);
void Lcm_Delayms(u32 n); 
void Lcm_WriteByte(u8 ch);
void Lcm_DishWriteByte(u8 ch);
void Lcm_DishInitInterface(u32 baudrate);
void Lcm_InitTxDma(u8 *pTxBuffer, u32 len);
void Lcm_InitRxDma(u8 *pRxBuffer, u32 len);
void Lcm_DishConfigInt(FunctionalState state);
void Lcm_DishWriteBuffer(u8 *pFrame, u16 len);
void Lcm_DishEnableInt(FunctionalState rxState, FunctionalState txState);

#endif
