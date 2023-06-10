#include "AnyID_CS1110_SysCfg.h"

int main(void)
{
    Sys_Init();
    while(1)
    {
        Sys_SoundTask();
        Sys_KeyTask();
        Sys_GPBTask();
        Sys_RfidTask();
        Sys_LedTask();
        Sys_UartTask();
        Sys_LcmTask();
        Sys_TestTask();
        Sys_ReaderTask();
    }
}
