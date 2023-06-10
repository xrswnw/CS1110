#ifndef _ANYID_CS1110_FLASH_H
#define _ANYID_CS1110_FLASH_H

#include "AnyID_CS1110_Config.h"

#define FRAM_BOOT_APP_OK                    0x5555
#define FRAM_BOOT_APP_FAIL                  0xAAAA

#define FRAM_BR_9600                        0
#define FRAM_BR_38400                       1
#define FRAM_BR_115200                      2
typedef struct framBootDevicePar{
    u16 appState;
    u16 br;
    u16 rfu;
    u16 crc;
}FRAM_BOOTPARAMS;
extern FRAM_BOOTPARAMS g_sFramBootParamenter;
//128KµÄFALSH
#define FRAME_BOOT_INFO_BACKUP_ADDR         (0x0800F000)
#define FRAME_BOOT_INFO_ADDR                (0x0800F800)
#define FRAME_INFO_ADDR                     (0x0800FC00)
#define FRAME_INFO_BACKUP_ADDR              (0x0800F400)

#define FRAME_SN_SIZE                       200

BOOL FRam_ReadBuffer(u32 addr, u16 len, u8 *pBuffer);
BOOL FRam_WriteBuffer(u32 addr, u16 len, u8 *pBuffer);
void FRam_Delayms(u32 n);

typedef struct framDeviceParams{
    u16 br;
    u16 rfu0;
    u16 rfu1;
    u16 crc;
}FRAM_DEVPARAMS;
extern FRAM_DEVPARAMS g_sFramDevParams;

void FRam_InitInterface(void);
void Fram_ReadBootParamenter(void);
BOOL Fram_WriteBootParamenter(void);
BOOL Fram_WriteBackupBootParamenter(void);
u32 Fram_GetUartBaudrate(void);

void Fram_ReadDevParams();
void Fram_WriteDevParams(void);
#endif

