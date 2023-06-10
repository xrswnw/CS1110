#ifndef _ANYID_CS1110_FLASHM25_
#define _ANYID_CS1110_FLASHM25_
#include "AnyID_CS1110_Reader.h"
#include "AnyID_FlashM25.h"
#define FLASH_PROGRAMM_TIMEOUT      1000
extern u32 g_nFlashPrgDelay;
typedef struct flashPrgInfo{
    u32 addr;
    u32 size;
    u32 index;
    u8 *pBuffer;
}FLASH_PRGINFO;
extern FLASH_PRGINFO g_sFlashPrgInfo;
#define FLASH_READ_BUFFER_LEN       2048
extern u8 g_aFashBuffer[FLASH_READ_BUFFER_LEN];


//
#define FLASH_OFFLINE_DATA_LEN          50

typedef struct flashOffLineDatas{
    u16 id;
    u16 len;
    u8 pBuffer[FLASH_OFFLINE_DATA_LEN];
    u32 crc32;
}FLASH_DATA;
extern FLASH_DATA g_sFlashData;
extern u8 FlashTempData[FLASH_OFFLINE_DATA_LEN] ;
//
u8 Flash_ReadStatus(void);
void Flash_InitProgrammInfo(u8 *pBuffer, u32 addr, u32 size);
BOOL Flash_SaveInfo(FLASH_DATA *pInfo);
void Flash_Demo();
#endif

