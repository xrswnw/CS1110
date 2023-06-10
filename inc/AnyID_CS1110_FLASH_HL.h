#ifndef _ANYID_CS1110_FLASH_HL_H
#define _ANYID_CS1110_FLASH_HL_H

#include "AnyID_CS1110_Config.h"

extern const PORT_INF FLASH_CS_COM;
#define Flash_CsLow()           FLASH_CS_COM.Port->BRR = FLASH_CS_COM.Pin
#define Flash_CsHigh()          FLASH_CS_COM.Port->BSRR = FLASH_CS_COM.Pin

#define FLASH_PORT              SPI2
#define FLASH_SPI_DUMMY         0xFF

void Flash_InitInterface(void);
u8 Flash_WriteByte(u8 byte);
u8 Flash_ReadByte(void);
void Flash_Delayms(u32 n);
void Flash_Delay16us(u32 us);

#define FLASH_PAGE_SIZE         512
#define FLASH_PAGE_MASK         0x1FF
#define FLASH_SECTOR_MASK       0x3FFFF

#define FLASH_SECTOR_NUM        256
#define FLASH_SECTOR_SIZE       0x40000

#define FRAME_BOOT_INFO_BACKUP_ADDR         (0x0100F000)
#define FRAME_BOOT_INFO_ADDR                (0x0100F800)
#define FRAME_INFO_ADDR                     (0x0100FC00)
#define FRAME_INFO_BACKUP_ADDR              (0x0100F400)

#define Flash_GetSectorAddr(i)  ((i) << 18)


#endif

