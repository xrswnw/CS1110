#ifndef _ANYID_CS1110_FRAM_H
#define _ANYID_CS1110_FRAM_H

#include "AnyID_FRam.h"

#define FRAM_BOOT_APP_OK                    0x5555
#define FRAM_BOOT_APP_FAIL                  0xAAAA

#define FRAM_BR_9600                        0
#define FRAM_BR_38400                       1
#define FRAM_BR_115200                      2

typedef struct framBootDevicePar{
    u16 appState;
    u16 br;
    u16 addr;
    u16 crc;
}FRAM_BOOTPARAMS;
extern FRAM_BOOTPARAMS g_sFramBootParams;


#define FRAM_OFFLINE_DATA_LEN          32

#define FRAM_OFFLINE_DATA              128
typedef struct framOffLineDatas{
    u16 id;
    u16 len;
    u8 pBuffer[FRAM_OFFLINE_DATA_LEN];
}FRAM_DATA;
extern FRAM_DATA g_sFramData;

#define FRAM_MEM_SIZE                       (32 << 10)    //32k
#define FRAM_BOOT_INFO_BACKUP_ADDR          (FRAM_MEM_SIZE - 512) 
#define FRAM_BOOT_INFO_ADDR                 (FRAM_MEM_SIZE - sizeof(FRAM_BOOTPARAMS))  

#define FRAM_SYS_INFO_ADDR                  0x0000    //存储系统参数--256
#define FRAM_SYS_INFO_BACKUP_ADDR           0x0100    //存储系统参数--256

#define FRAM_OFF_LINE_ADDR                  1024    //离线数据存储
#define FRAM_OFF_LINE_NUM_ADDR              400    //离线数据存储

void Fram_ReadBootParams(void);
BOOL Fram_WriteBootParams(void);
BOOL Fram_WriteBackupBootParams(void);
u32 Fram_GetUartBaudrate(void);

#endif