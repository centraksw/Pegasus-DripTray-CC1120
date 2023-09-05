#include <string.h>
#include "defines.h"
#include "flash_drv.h"
#include "timer_drv.h"

#define SEGB                            0x1000
#define SEGA                            0x1080
#define SEGMENT_SIZE                    0x80
/****************************************
**
** Flash_Read
**
*****************************************/
VOID FLASH_Read(VOID *data, WORD Addr, BYTE nBytes)
{
    memcpy((char *)data, (char *)Addr, nBytes);
}

/****************************************
**
** Clear the Segment(128 bytes)
**
*****************************************/
VOID FLASH_Clear(char *Addr)
{
    FCTL3 = FWKEY;
    FCTL1 = FWKEY + ERASE;

    while(FCTL3 & BUSY);

    *Addr = 0;

    while(FCTL3 & BUSY);

    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;

    while(FCTL3 & BUSY);
}

/****************************************
**
** Write to the Particular Address
**
*****************************************/
VOID FLASH_Write(VOID *data, WORD Addr, BYTE nBytes)
{
    FCTL3 = FWKEY;		// Unlock
    FCTL1 = FWKEY + WRT;	// Writing

    while(FCTL3 & BUSY);

    memcpy((char *)Addr, (char *)data, nBytes);

    while(FCTL3 & BUSY);

    FCTL1 = FWKEY;    		// Stop Writing
    FCTL3 = FWKEY + LOCK;	// Lock , Not ready for Next Word Write

    while(FCTL3 & BUSY);
}

VOID FLASH_Erase_Segment(char* addr)
{
    //Disable All interrupts
    _DINT();

    //Clear Lock
    FCTL3 = FWKEY;

    //Enable segment erase
    FCTL1 = FWKEY + ERASE;

    while(FCTL3 & BUSY);

    //Perform Erase
    *addr = 0;
    while(FCTL3 & BUSY);

    //Disable Erase
    FCTL1 = FWKEY;

    //set LOCK
    FCTL3 = FWKEY + LOCK;

    while(FCTL3 & BUSY);

    //Enable Interrupts
    _EINT();
}
