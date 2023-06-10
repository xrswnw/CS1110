#include "AnyID_CS1110_FlashM25.h"

#pragma location = ".ccmram"
u8 g_aFashBuffer[FLASH_READ_BUFFER_LEN] = {0};
FLASH_PRGINFO g_sFlashPrgInfo = {0};
u32 g_nFlashPrgDelay = 0;

//

FLASH_DATA g_sFlashData = {0};

u8 FlashTempData[FLASH_OFFLINE_DATA_LEN] = {0};

u8 Flash_ReadStatus(void)
{
    u8 state = 0;

    Flash_CsLow();
    Flash_WriteByte(FLASH_CMD_RDSR);
    state = Flash_WriteByte(0xFF);
    Flash_CsHigh();

    return state;
}

void Flash_InitProgrammInfo(u8 *pBuffer, u32 addr, u32 size)
{
    g_sFlashPrgInfo.addr = addr;
    g_sFlashPrgInfo.size = size;
    g_sFlashPrgInfo.pBuffer = pBuffer;
    g_sFlashPrgInfo.index = 0;
    Flash_EraseSector(addr);
}

BOOL Flash_SaveInfo(FLASH_DATA *pInfo)
{
    BOOL b = FALSE;

    if(pInfo->len <= FLASH_OFFLINE_DATA_LEN)
    {
        b = Flash_WritePage(pInfo->id, pInfo->len, pInfo->pBuffer);
    }

    return b;
}

void Flash_Demo()
{

    g_sFlashData.id = Reader_Seek_Id(g_sReaderOffLineInfo.readerOffLineData);
    g_sReaderOffLineInfo.num = Reader_Seek_Num(g_sReaderOffLineInfo.readerOffLineData);
    Flash_ReadBuffer(g_sFlashData.id *  FLASE_PAGE_SIZE, FLASH_OFFLINE_DATA_LEN, g_sFlashData.pBuffer);
    
    /*
    Flash_ReadBuffer(0x0000 * FLASE_PAGE_SIZE, FLASH_OFFLINE_DATA_LEN, g_sFlashData.pBuffer);
    Flash_ReadBuffer(0x0001 * FLASE_PAGE_SIZE, FLASH_OFFLINE_DATA_LEN, g_sFlashData.pBuffer);
    Flash_ReadBuffer(0x0002 * FLASE_PAGE_SIZE, FLASH_OFFLINE_DATA_LEN, g_sFlashData.pBuffer);
    Flash_ReadBuffer(0x0003 * FLASE_PAGE_SIZE, FLASH_OFFLINE_DATA_LEN, g_sFlashData.pBuffer);
    */

}

