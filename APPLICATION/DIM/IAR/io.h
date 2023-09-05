#ifndef __IO_H
#define __IO_H

#define LED_RED                     1
#define LED_GREEN                   2
#define LED_YELLOW                  3

VOID IO_ShortSleep_Enter();
VOID IO_ShortSleep_Exit();

VOID IO_TurnOffLED(BYTE color);
VOID IO_LEDLit(BYTE color, DWORD delay);
VOID IO_TurnOnLED(BYTE color);
BYTE IO_ScanKeys();
#endif
