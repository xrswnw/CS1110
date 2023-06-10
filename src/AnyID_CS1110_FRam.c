#include "AnyID_CS1110_FRam.h"

FRAM_BOOTPARAMS g_sFramBootParams = {0};
FRAM_DATA g_sFramData = {0};
void Fram_ReadBootParams(void)
{
    BOOL b = FALSE, bBackUp = FALSE;
    FRAM_BOOTPARAMS backupParams = {0};
    u8 repeat = 0;
    do{
        b = FRam_ReadBuffer(FRAM_BOOT_INFO_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&g_sFramBootParams));
        if(b)
        {
            u16 crc1 = 0, crc2 = 0;

            crc1 = a_GetCrc((u8 *)(&g_sFramBootParams), (sizeof(FRAM_BOOTPARAMS)) - 2);
            crc2 = g_sFramBootParams.crc;

            //检测参数是否正确，如果不正确，使用默认参数操作
            if(crc1 != crc2)
            {
                b = FALSE;
            }
            else
            {
                b = TRUE;
            }
            break;
        }
        else
        {
            FRam_Delayms(50);
            repeat++;
        }
    }while(repeat < 3);

    repeat = 0;
    do{
        bBackUp = FRam_ReadBuffer(FRAM_BOOT_INFO_BACKUP_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&backupParams));
        if(bBackUp)
        {
            u16 crc1 = 0, crc2 = 0;

            crc1 = a_GetCrc((u8 *)(&backupParams), (sizeof(FRAM_BOOTPARAMS)) - 2);
            crc2 = backupParams.crc;

            //检测参数是否正确，如果不正确，使用默认参数操作
            if(crc1 != crc2)
            {
                bBackUp = FALSE;
            }
            else
            {
                bBackUp = TRUE;
            }
            break;
        }
        else
        {
            FRam_Delayms(50);
            repeat++;
        }
    }while(repeat < 3);
    
    if(bBackUp == FALSE && b == FALSE)
    {
        g_sFramBootParams.appState = FRAM_BOOT_APP_FAIL;
        g_sFramBootParams.br = FRAM_BR_115200;
        
        Fram_WriteBootParams();
        Fram_WriteBackupBootParams();
    }
    else if(bBackUp == FALSE)
    {
        Fram_WriteBackupBootParams();
    }
    else if(b == FALSE)
    {
        memcpy(&g_sFramBootParams, &backupParams, sizeof(FRAM_BOOTPARAMS));
        Fram_WriteBootParams();
    }
    
}

BOOL Fram_WriteBootParams(void)
{
    BOOL b = FALSE;
    u8 repeat = 0;

    g_sFramBootParams.crc = 0;
    g_sFramBootParams.crc = a_GetCrc((u8 *)(&g_sFramBootParams), (sizeof(FRAM_BOOTPARAMS)) - 2);
    do{
        b = FRam_WriteBuffer(FRAM_BOOT_INFO_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&g_sFramBootParams));
        if(b)
        {
            b = FRam_WriteBuffer(FRAM_BOOT_INFO_BACKUP_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&g_sFramBootParams));
        }
        if(b == FALSE)
        {
            FRam_Delayms(50);
            repeat++;
        }
        else
        {
            break;
        }
    }while(repeat < 3);

    return b;
}

BOOL Fram_WriteBackupBootParams(void)
{
    BOOL b = FALSE;
    u8 repeat = 0;

    g_sFramBootParams.crc = 0;
    g_sFramBootParams.crc = a_GetCrc((u8 *)(&g_sFramBootParams), (sizeof(FRAM_BOOTPARAMS)) - 2);
    do{
        b = FRam_WriteBuffer(FRAM_BOOT_INFO_BACKUP_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&g_sFramBootParams));
        if(b == FALSE)
        {
            FRam_Delayms(50);
            repeat++;
        }
        else
        {
            break;
        }
    }while(repeat < 3);

    return b;
}


