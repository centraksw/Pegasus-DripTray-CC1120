#include "defines.h"
#include "timer_drv.h"
#include "io.h"

VOID IO_ShortSleep_Exit()
{
    P2DIR &= ~OSC_PIN; // ROsc
}

VOID IO_ShortSleep_Enter()
{

    /* PORT 1

    BIT0 - I2C DATA
    BIT1 - I2C CLK
    BIT2 - LF DATA
    BIT3 - RF Reset
    BIT4 - JTAG
    BIT5 - JTAG
    BIT6 - JTAG
    BIT7 - Unsued  */

    /* PORT 2

    BIT0 - Antenna1
    BIT1 - Antenna2
    BIT2 - LBI pin going to A/D
    BIT3 - GREEN LED
    BIT4 - Motion Sensor
    BIT5 - ROSC
    BIT6 - 32 KHz Crystal
    BIT7 - 32 KHz Crystal  */

    /* PORT 3

    BIT0 - RF - CSN
    BIT1 - RF - SI
    BIT2 - RF - SO
    BIT3 - RF - SCLK
    BIT4 - RF - GD02
    BIT5 - RF - GDO0
    BIT6 - RED LED
    BIT7 - ADC Input (Battery Status)  */

    /* PORT 4

    BIT0 - Step UP Convertor
    BIT1 - Unsued
    BIT2 - LF - Envelop Signal
    BIT3 - Unsued
    BIT4 - Unsued
    BIT5 - Unsued
    BIT6 - Unsued
    BIT7 - Unsued */

    BYTE red_led_dir;
    BYTE red_led_out;
    BYTE Green_led_dir;
    BYTE Green_led_out;
#ifdef __ENABLE_DEBUG__
    BYTE olddir = P1DIR & (BIT5 | BIT6);
    BYTE oldout = P1OUT & (BIT5 | BIT6);
    P1DIR = UNUSED_PIN7_INV | olddir | JTAG_PIN3     | RF_RESET | LF_DATA_INV | I2C_CLK_INV | I2C_DATA_INV;
    P1OUT = UNUSED_PIN7_INV | oldout | JTAG_PIN3_INV | RF_RESET | LF_DATA_INV | I2C_CLK_INV | I2C_DATA_INV;
#else
    P1DIR = UNUSED_PIN7_INV | JTAG_PIN1     | JTAG_PIN2     | JTAG_PIN3     | RF_RESET | LF_DATA | I2C_CLK_INV | I2C_DATA_INV;
    P1OUT = UNUSED_PIN7_INV | JTAG_PIN1_INV | JTAG_PIN2_INV | JTAG_PIN3_INV | RF_RESET | LF_DATA_INV | I2C_CLK_INV | I2C_DATA_INV;
#endif
    Green_led_dir = P2DIR & LED_GREEN_PIN;
    Green_led_out = P2OUT & LED_GREEN_PIN;

    P2DIR = CRYSTAL_32KHZ_PIN1_INV | CRYSTAL_32KHZ_PIN2_INV | OSC_PIN_INV | MOTION_SENSOR_INV | Green_led_dir | BATTERY_ADC_INPUT_INV | RF_ANTENNA2     | RF_ANTENNA1;
    P2OUT = CRYSTAL_32KHZ_PIN1_INV | CRYSTAL_32KHZ_PIN2_INV | OSC_PIN_INV | MOTION_SENSOR_INV | Green_led_out | BATTERY_ADC_INPUT_INV | RF_ANTENNA2_INV | RF_ANTENNA1_INV;

    red_led_dir = P3DIR & LED_RED_PIN;
    red_led_out = P3OUT & LED_RED_PIN;

    P3DIR = BATTERY_ADC_POWER_INV | red_led_dir | RF_GDO0_INV | RF_GDO2_INV | RF_SCLK | RF_SO_INV | RF_SI | RF_CSN;
    P3OUT = BATTERY_ADC_POWER_INV | red_led_out | RF_GDO0_INV | RF_GDO2_INV | RF_SCLK | RF_SO_INV | RF_SI | RF_CSN;

    P4DIR = BTN1_PIN_INV | UNUSED_PIN6_INV | UNUSED_PIN5_INV | UNUSED_PIN4_INV | UNUSED_PIN3_INV | LF_ENV_SIGNAL     | UNUSED_PIN1_INV | LF_STEPUP_CONV_INV;
    P4OUT = BTN1_PIN_INV | UNUSED_PIN6_INV | UNUSED_PIN5_INV | UNUSED_PIN4_INV | UNUSED_PIN3_INV | LF_ENV_SIGNAL_INV | UNUSED_PIN1_INV | LF_STEPUP_CONV_INV;
}


VOID IO_TurnOnLED(BYTE color)
{
    // Turn On LED
    if( color & LED_RED )
    {
        LED_RED_DIR |= LED_RED_PIN;
        LED_RED_OUT |= LED_RED_PIN;
    }
    if( color & LED_GREEN )
    {
        LED_GREEN_DIR |= LED_GREEN_PIN;
        LED_GREEN_OUT |= LED_GREEN_PIN;
    }
}

VOID IO_TurnOffLED(BYTE color)
{
    // Turn Off LED
    if( color & LED_RED )
    {
        LED_RED_DIR |= LED_RED_PIN;
        LED_RED_OUT &= ~LED_RED_PIN;
    }
    if( color & LED_GREEN )
    {
        LED_GREEN_DIR |= LED_GREEN_PIN;
        LED_GREEN_OUT &= ~LED_GREEN_PIN;
    }
}

VOID IO_LEDLit(BYTE color, DWORD delay)
{
    IO_TurnOnLED(color);

    TIMER_DelayMS(delay);

    IO_TurnOffLED(color);
}

BYTE IO_ScanKeys()
{
#ifdef __EXTERNAL_DIM__
    return 0;
#endif
    BYTE keys;

    BTN_DIR  &= ~BTN1_PIN;
    BTN1_REN |= BTN1_PIN;
    BTN_OUT  |= BTN1_PIN;

    TIMER_DelayUS(50);

    keys = BTN_IN;

    BTN1_REN &= ~BTN1_PIN;

    if((keys & BTN1_PIN) == 0)
        return 1;

    return 0;
}
