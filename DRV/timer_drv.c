#include <stdlib.h>
#include "defines.h"
#include "timer_drv.h"
#include "general.h"

BYTE curSlot;

WORD g_PageCounter;
WORD g_LocationCounter;
WORD g_UpgradeLocationCounter;
WORD g_BatteryCounter;
DWORD g_SummaryCounter;
DWORD g_SummaryWriteCounter;
static WORD tbr_initial_cnt;
static BOOL blnInCCRDelay = FALSE;

VOID MICRO_Sleep_LPM1()
{
    blnInCCRDelay = TRUE;
    _BIS_SR(LPM1_bits + GIE);
    blnInCCRDelay = FALSE;
}

BYTE DCO = 0x90;
BYTE BCS = 0x8B;

static VOID Set_DCO()
{
#define DELTA  126
    unsigned int Compare, Oldcapture = 0;
    WORD counter2 = 0;
    WORD counter1 = 0;

    TACCTL2 = CM_1 + CCIS_1 + CAP + SCS;            // CAP, ACLK
    TACTL = TASSEL_2 + MC_2 + TACLR;          // SMCLK, cont-mode, clear
 
    BCSCTL2 &= ~SELS;
    while (1)
    {
        counter1 = RESET_TIMEOUT;
        while (--counter1 && !(CCIFG & TACCTL2));             // Wait until capture occured
        if( counter1 == 0 )
        {
      		BCSCTL1 = BCS;
      		DCOCTL = DCO;
            break;
        }
        TACCTL2 &= ~CCIFG;                      // Capture occured, clear flag
        Compare = TACCR2;                       // Get current captured SMCLK
        Compare = Compare - Oldcapture;         // SMCLK difference
        Oldcapture = TACCR2;                    // Save current captured SMCLK

        // If DCO is not set within 1000 tries,
        // Exit SET DCO call with default values
        if (++counter2 > 1000)
        {
      		BCSCTL1 = BCS;
      		DCOCTL = DCO;
            break;
        }
        // Exit from DCO call if DCO is set.
	    if (DELTA == Compare)
	    {
	      BCS = BCSCTL1;
	      DCO = DCOCTL;
	      break;            // If equal, leave "while(1)"
	    }
        else if (DELTA < Compare)               // DCO is too fast, slow it down
        {
            DCOCTL--;
            if (DCOCTL == 0xFF)
            {
                if( (BCSCTL1 & 7) > 0 )
                    BCSCTL1--;                  // Did DCO roll under?, Sel lower RSEL
            }
        }
        else
        {
            DCOCTL++;
            if (DCOCTL == 0x00)
            {
                if( (BCSCTL1 & 7) < 7 )
                    BCSCTL1++;                  // Did DCO roll over? Sel higher RSEL
            }
        }
    }
    TACCTL2 = 0;                              // Stop TACCR2
    TACTL = 0;                                // Stop Timer_A
}

VOID TIMER_MicroInit()
{
    // Stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;
    
    BCSCTL1 = 0x8B;
    DCOCTL =  0x90;
}

VOID TIMER_Init()
{
    Set_DCO();

    // ACLK, Continous mode, each tick=30.51 us
    TACTL = TACLR + TASSEL_1 + MC_2;
    TBCTL = TBSSEL_1 + MC_2;
}

// Each tick is 30.51 us
// Micro will be in LPM1 mode.
// LPM3 micro wakeup execuited when the chip is in LPM1.
// Min Delay: 30.51 us
// Max Delay: 1.999472.85 secs
VOID TIMER_CCR_Delay(WORD time)
{
    if( time <= 1 ) return;

    TACTL &= ~MC_2;
    TACCTL0 |= CAP;   // TA12 fix: put TA into capture mode
    TACCR0 = TAR + time;
    TACCTL0 &= ~CAP; // TA12 fix: put TA back into compare mode
    TACTL |= MC_2;

    TACCTL0 |= CCIE;

    blnInCCRDelay = TRUE;
    _BIS_SR(LPM3_bits + GIE);
    blnInCCRDelay = FALSE;

    _NOP();

    // Disable interrupt
    TACCTL0 = 0;
}

// This is a service function, which calls the CCR_Delay
// We should give delay value in micro seconds.
// this function divides the delay value  by 30.52 and send it to CCR_Delay
VOID TIMER_DelayUS(DWORD delay)
{
    delay &= 0xFFFFF;
    if( delay <= 1 ) return;

    delay *= 100;
    TIMER_CCR_Delay(delay / 3052);
}


// This is a service function, which calls the CCR_Delay
// We should give delay value in milli seconds.
VOID TIMER_DelayMS(WORD delay)
{
    TIMER_DelayUS(delay * 1000l);
}


// This will put the micro to LPM3 mode sleep for the specified amount of time.
// Sleep time should be in multiples of 250 ms.
VOID TIMER_SleepSlot(WORD time)
{
    WORD idx;
    if( time == 0 ) return;
    TIMER_InitWDT();
    for( idx=1; idx<=time; idx++ )
    {
        Micro_Sleep();
    }
    TIMER_StopWDT();
}


// Initialize WDT
// WDT will be triggered once in every 250ms.
VOID TIMER_InitWDT()
{
  //  DBG_TOGGLE_TIMER_PIN();

    WDTCTL = WDT_ADLY_250;

    TBCTL = TBSSEL_1 + MC_2;                    // ACLK, up mode
    IE1 |= WDTIE;                               // Enable WDT interrupt
}

// Stop WDT
VOID TIMER_StopWDT()
{
    TBCTL = 0;
    IE1 &= (~WDTIE);
    WDTCTL = WDTPW + WDTHOLD;
}


// Initialize the slot offset counter.
// which is used to measure how much time elapsed after last WDT trigger.
VOID TIMER_InitSlotOffset()
{
    tbr_initial_cnt = TBR;
}

DWORD TIMER_GetSlotOffsetInUS()
{
    DWORD time;
    time = TBR;
    if ( time < tbr_initial_cnt )
        time += 65536;
    time = time - tbr_initial_cnt;
    time *= 3052l;
    time /= 100; return time;
}




// Watchdog Timer interrupt service routine
#pragma vector = WDT_VECTOR
__interrupt VOID watchdog_timer()
{

    DBG_TOGGLE_TIMER_PIN();

    //We should not wakeup micro if it is in LPM1 mode sleep.
    if( !blnInCCRDelay )
        Micro_Wakeup();

    // Increment slot counter and check it exceeds max slots,
    // then reset curslot to initial value : 1
    ++curSlot;
	if( curSlot > MAX_SLOTS )
        curSlot = 1;


    ++g_PageCounter;
    ++g_LocationCounter;
    ++g_BatteryCounter;
    ++g_SummaryWriteCounter;
    ++g_SummaryCounter;

    ++g_UpgradeLocationCounter;

}

// IR Receive
#pragma vector = TIMER0_A0_VECTOR
__interrupt VOID TIMERA0_CCR0()
{
    Micro_Wakeup();
}

#pragma vector = TIMERB1_VECTOR
__interrupt VOID TB1_ISR()
{
    switch(TBIV)
    {
    case 2:
        TBCCTL1 &= ~CCIFG;
        MICRO_Wakeup_LPM1();
        break;
    }
}