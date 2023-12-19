#ifndef __LF_H__
#define __LF_H__

#define GETBIT(byte, bit) ( (byte >> bit) & 7 )

VOID LF_Initialize();
VOID LF_Transmit(WORD irData,BYTE LFRange);
VOID EnableLEDTimer();
VOID DisableLEDTimer();

#endif