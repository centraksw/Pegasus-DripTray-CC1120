#ifndef __TIMER_DRV_H
#define __TIMER_DRV_H

#define Micro_Sleep()               _BIS_SR(LPM3_bits + GIE)
#define Micro_Wakeup()              _BIC_SR_IRQ(LPM3_bits)
#define MICRO_Wakeup_LPM1()         _BIC_SR_IRQ(LPM1_bits)

extern BYTE curSlot;

extern WORD g_PageCounter;
extern WORD g_LocationCounter;
extern WORD g_BatteryCounter;
extern DWORD g_SummaryCounter;
extern DWORD g_SummaryWriteCounter;
extern WORD g_UpgradeLocationCounter;

VOID MICRO_Sleep_LPM1();

VOID TIMER_MicroInit();

VOID TIMER_Init();

VOID TIMER_CCR_Delay(WORD time);

VOID TIMER_SleepSlot(WORD time);

VOID TIMER_InitWDT();

VOID TIMER_StopWDT();

VOID TIMER_DelayUS(DWORD delay);

VOID TIMER_DelayMS(WORD delay);

VOID TIMER_InitSlotOffset();

DWORD TIMER_GetSlotOffsetInUS();

#endif // __TIMER_DRV_H
