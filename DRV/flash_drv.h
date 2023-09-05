#ifndef __FLASH_H
#define __FLASH_H

VOID FLASH_Read(VOID *data, WORD Addr, BYTE nBytes);
VOID FLASH_Clear(char *Addr);
VOID FLASH_Write(VOID *data, WORD Addr, BYTE nBytes);
VOID FLASH_Erase_Segment(char* addr);
#endif
