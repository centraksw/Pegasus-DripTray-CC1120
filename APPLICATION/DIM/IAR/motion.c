#include "defines.h"
#include "motion.h"
#include "flash_app.h"

static BYTE MDCount=0;

VOID MD_Init()
{
    MD_INT_FLAG = 0;

    MD_DIR &= ~MD_PIN2;
    MD_INT |= MD_PIN2;
    P2IES |= MD_PIN2;

    _EINT();
}

// Turn off the Motion sensor interrutpt
// Clear the interrupt flag
VOID MD_Disable()
{
    MD_INT_FLAG = 0;
    MD_INT &= ~MD_PIN2;
}

// Enable the motion sensor interrupt
VOID MD_Enable()
{
    MD_INT_FLAG = 0;
    MD_INT |= MD_PIN2;
    _EINT();
}

BOOL MD_Triggered()
{
    BOOL blnRes = FALSE;
    if( MDCount > flash_settings.MDMaxCount )
        blnRes = TRUE;
    MDCount = 0;
    return blnRes;
}

/***** Button Interrupt *****/
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    // Check is motion interrupt triggered.
    if( MD_INT_FLAG & MD_PIN2 )
        MDCount += 1;

    // Clear Interrupt flag
    MD_INT_FLAG = 0;
}
