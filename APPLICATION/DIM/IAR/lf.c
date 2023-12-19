#include "defines.h"
#include "pinconfig.h"
#include "lf.h"
#include "timer_drv.h"
#include "io.h"

#define MARK_SIZE                         28
#define END_BIT                           80

static WORD LF_BIT_VALUE[] = {47, 109, 175, 245, 310, 378, 444, 515};

static VOID OutputHigh(WORD val)
{
    WORD idx;                                   // Declare counter variable

    LF_ENV_SIGNAL_OUT |= LF_ENV_SIGNAL;

    TACCTL1 = OUTMOD_7;                         // CCR1 reset/set
    TACCR0 = 32;                                // PWM Period ~ 38kHz
    TACTL = TASSEL1 + TACLR;                    // SMCLK, Clear TA
    TACTL |= MC0;                               // Start TA in up mode
    for (idx=val; idx>0; idx--)                 // Count val interrupts
    {
        LPM0;                                   // at 40KHz each interrupt = ~25us
    }                                           // at 38KHz each interrupt = ~26us
    LF_ENV_SIGNAL_OUT &= ~LF_ENV_SIGNAL;
}

static VOID OutputLow(WORD val)
{
    WORD idx;                                   // Declare counter variable
    TACCTL1 = OUTMOD_0;                         // Set output of CC1 to 0
    TACCR0 = 32;                                // PWM Period ~ 38kHz
    TACTL = TASSEL1 + TACLR;                    // SMCLK, Clear TA
    TACTL |= MC0;                               // Start TA in up mode
    for (idx=val; idx>0; idx--)                 // Count val interrupts
    {
        LPM0;                                   // at 40KHz each interrupt = ~25us
    }                                           // at 38KHz each interrupt = ~26us
}

VOID LF_Initialize()
{
    LF_DATA_DIR |= LF_DATA;                     // All P2 outputs
    LF_DATA_SEL |= LF_DATA;                     // P2.3 TA1 option
    LF_DATA_OUT &= ~LF_DATA;                    // Clear P2 outputs
    _EINT();                                    // Enable interrupts
}

VOID LF_Transmit(WORD irData,BYTE LFRange)
{
    INT8 idx;
    TACTL = TACLR;                            // Stop and clear TA
    TACCR1 = LFRange;                         // CCR1 PWM Duty Cycle ~26%
    TACCTL0 = CCIE;                           // Enable CCR0 interrupt

    // Configure Gate control
    LF_DATA_DIR |= LF_DATA;
    //LF_ENV_SIGNAL_DIR |= LF_ENV_SIGNAL;    // TAL

    // Turn on Signal(P1.2), DC (P1.3)
    LF_DATA_OUT |= LF_DATA;

    // Carrier Burst
    OutputHigh(250);

    // 5 ms gap
    OutputLow(625);

    // GAP
    OutputLow(75);

    OutputLow(38);

    OutputHigh(MARK_SIZE);
    OutputLow(393);

    // Start bit
    if( LFRange == LF_RANGE_HIGH )
    OutputHigh(71);
    else
        OutputHigh(77);

    for(idx=9; idx>=0; idx-=3)
    {
        OutputLow(LF_BIT_VALUE[GETBIT(irData, idx)]);
        OutputHigh( idx ? MARK_SIZE : END_BIT );
    }

    TACCTL1 = 0;
    TACCTL0 = 0;

    // Turn off Signal, DC
    LF_DATA_OUT &= ~LF_DATA;
  // LF_ENV_SIGNAL_OUT &= ~LF_ENV_SIGNAL;
}
static BOOL blnTurnOnLED = FALSE;
VOID EnableLEDTimer()
{
    blnTurnOnLED = TRUE;
    TA1CCTL1 = 0;
    TA1CTL = TASSEL_1 + MC_2;
    TA1CCR1 = TA1R + 16388;
    TA1CCTL1 |= CCIE;
    IO_TurnOnLED(LED_GREEN);
}

VOID DisableLEDTimer()
{
    TA1CCTL1 = 0;
    blnTurnOnLED = FALSE;
    IO_TurnOffLED(LED_GREEN);
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer0_ISR(void)
{
    if( TAIV == 2)
        Micro_Wakeup();
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer1_ISR(void)
{
    if( TA1IV == 2)
    {
        if(blnTurnOnLED)
            DisableLEDTimer();
    }
}
