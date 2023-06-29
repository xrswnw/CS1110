#include "AnyID_CS1110_Key.h"

KEY_VALUE g_sKeyValue = {0};

void Key_Init()
{
    Key_InitInterface();
    Key_Low_High();
    g_sKeyValue.state = KEY_STAT_IDLE;
}


void Key_Control(KEY_VALUE *pBuffer)
{
    if(pBuffer->mode == KEY_MDOE_TEST)
    {
        Key_Value_Test(pBuffer);
    }
    else 
    {
        Key_Value_Main(pBuffer);
    }
}

void Key_Value_Main(KEY_VALUE *pBuffer)
{
    switch(pBuffer->value)
    {
      case KEY_VALUE_LEFT:

        
        break;
        case KEY_VALUE_MID:
        
        
        break;
        case KEY_VALUE_RIGHT:
        
        
        break;
        case KEY_VALUE_LONG_LEFT:
   
        break;
        case KEY_VALUE_LONG_MID:

        
        break;
        case KEY_VALUE_LONG_RIGHT:
            
        
        break;
        case (0 | KEY_VALUE_LONG_MID | KEY_VALUE_LONG_RIGHT):
   
        break;
        case (KEY_VALUE_LONG_LEFT | KEY_VALUE_LONG_MID | KEY_VALUE_LONG_RIGHT):
            g_sDeviceParamenter.reWorkMode = READER_MODE_TEST;
            pBuffer->mode = KEY_MDOE_TEST;
            pBuffer->value = 0;
            g_sReaderRfidTempInfo.okTick = 0;
            g_sReaderRfidTempInfo.tick = 0; 
            g_sReaderRfidTempInfo.succesTick = 0;
            g_sReaderRfidTempInfo.allTick = 0; 
            Lcm_SetPage(LCM_FLAG_PAGE_TEST );    
            
        break;
        case (KEY_VALUE_LONG_LEFT | 0 | KEY_VALUE_LONG_RIGHT):

          
        break;
    }	
}


void Key_Value_Test(KEY_VALUE *pBuffer)
{
    switch(pBuffer->value)
    {
      case KEY_VALUE_LEFT:

        break;
        case KEY_VALUE_MID:
        
        
        break;
        case KEY_VALUE_RIGHT:
        
        
        break;
        case KEY_VALUE_LONG_LEFT:
          
            
        break;
        case KEY_VALUE_LONG_MID:

        
        break;
        case KEY_VALUE_LONG_RIGHT:

           pBuffer->value = 0;
            g_sGpbInfo.mode = GPB_WORK_SET_ZERO; 
        break;
        case (KEY_VALUE_LONG_MID | KEY_VALUE_LONG_RIGHT):
          

            break;
        case (KEY_VALUE_LONG_LEFT | KEY_VALUE_LONG_MID | KEY_VALUE_LONG_RIGHT):
            g_sDeviceParamenter.reWorkMode = READER_MODE_NORMAL;
            pBuffer->mode = KEY_MDOE_MAIN;
            pBuffer->value = 0;
            g_sReaderRfidTempInfo.okTick = 0;
            g_sReaderRfidTempInfo.tick = 0; 
            g_sReaderRfidTempInfo.succesTick = 0;
            g_sReaderRfidTempInfo.allTick = 0; 
            Lcm_SetPage(LCM_PAGE_NULL_WHITE + g_sDeviceParamenter.uiMode);    
        break;
        case (KEY_VALUE_LONG_LEFT | 0 | KEY_VALUE_LONG_RIGHT):
            pBuffer->value = 0;
            g_sReaderRfidTempInfo.okTick = 0;
            g_sReaderRfidTempInfo.tick = 0; 
            g_sReaderRfidTempInfo.succesTick = 0;
            g_sReaderRfidTempInfo.allTick = 0; 
            Reader_DisplayTest(&g_sRaderInfo.dishInfo , &g_sWigthInfo);
        break;
    }	
}
