#include "AnyID_CS1110_Key_HL.h"

const PORT_INF KEY_POW_PORT = {GPIOB, GPIO_Pin_1};

const PORT_INF KEY_PORT0 = {GPIOC, GPIO_Pin_6};
const PORT_INF KEY_PORT1 = {GPIOC, GPIO_Pin_7};
const PORT_INF KEY_PORT2 = {GPIOC, GPIO_Pin_8};
void Key_InitInterface()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = KEY_PORT0.Pin;
    GPIO_Init(KEY_PORT0.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY_PORT1.Pin;
    GPIO_Init(KEY_PORT1.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY_PORT2.Pin;
    GPIO_Init(KEY_PORT2.Port, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = KEY_POW_PORT.Pin;
    GPIO_Init(KEY_POW_PORT.Port, &GPIO_InitStructure);
	

}
    
u8 Key_GetValue()
{
    u8 k = 0x00;
    
    if(((~KEY_PORT0.Port->IDR) & KEY_PORT0.Pin))
    {
        k |= 0x04;
    }
    else
    {
        k &= ~0x04; 
    }
    if(((~KEY_PORT1.Port->IDR) & KEY_PORT1.Pin))
    {
        k |= 0x02;
    }
    else
    {
        k &= ~0x02; 
    }
    if(((~KEY_PORT2.Port->IDR) & KEY_PORT2.Pin))
    {
        k |= 0x01;
    }
    else
    {
        k &= ~0x01; 
    }
    
  

    return k;
}

