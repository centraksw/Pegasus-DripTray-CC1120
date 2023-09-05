#ifndef __PINCONFIG_H
#define __PINCONFIG_H

/*
P3.0	- CSn
P3.1	- SI
P3.2	- SO
P3.3	- SClk
P3.4	- GDO2
P3.5	- GDO0
*/
#define RF_CSN_SEL              P3SEL
#define RF_CSN_DIR              P3DIR
#define RF_CSN_OUT              P3OUT
#define RF_CSN_IN               P3IN
#define RF_CSN                  BIT0

#define RF_SI_SEL               P3SEL
#define RF_SI_DIR               P3DIR
#define RF_SI_OUT               P3OUT
#define RF_SI_IN                P3IN
#define RF_SI                   BIT1

#define RF_SO_SEL               P3SEL
#define RF_SO_DIR               P3DIR
#define RF_SO_OUT               P3OUT
#define RF_SO_IN                P3IN
#define RF_SO                   BIT2
#define RF_SO_INV               0

#define RF_SCLK_SEL             P3SEL
#define RF_SCLK_DIR             P3DIR
#define RF_SCLK_OUT             P3OUT
#define RF_SCLK_IN              P3IN
#define RF_SCLK                 BIT3

#define RF_GDO2_SEL             P3SEL
#define RF_GDO2_DIR             P3DIR
#define RF_GDO2_OUT             P3OUT
#define RF_GDO2_IN              P3IN
#define RF_GDO2                 BIT4
#define RF_GDO2_INV             0

#define RF_GDO0_SEL             P3SEL
#define RF_GDO0_DIR             P3DIR
#define RF_GDO0_OUT             P3OUT
#define RF_GDO0_IN              P3IN
#define RF_GDO0                 BIT5
#define RF_GDO0_INV             0

//PORT 1
#define JTAG_PIN1               BIT6
#define JTAG_PIN1_INV           0

#define JTAG_PIN2               BIT5
#define JTAG_PIN2_INV           0

#define JTAG_PIN3               BIT4
#define JTAG_PIN3_INV           0

#define LF_DATA_INV             0
#define I2C_CLK_INV             0
#define I2C_DATA_INV            0
#define BTN1_PIN_INV            0

#define I2C_SEL                 P1SEL // Port selection
#define I2C_DIR                 P1DIR // Port direction
#define I2C_OUT                 P1OUT // Port output
#define I2C_IN                  P1IN  // Port input

#define I2C_SDA                 BIT0 // Controls SDA line (pull-up used for logic 1)
#define I2C_SCL                 BIT1 // Controls SCL line (pull-up used for logic 1)

#define LF_DATA_DIR             P1DIR
#define LF_DATA_SEL             P1SEL
#define LF_DATA_OUT             P1OUT
#define LF_DATA                 BIT2
#define RF_RESET_DIR            P1DIR
#define RF_RESET_OUT            P1OUT
#define RF_RESET                BIT3

//PORT2
#define CRYSTAL_32KHZ_PIN1_INV  0
#define CRYSTAL_32KHZ_PIN2_INV  0
#define OSC_PIN_INV             0
#define MOTION_SENSOR_INV       0
#define LED_GREEN_INV           0
#define BATTERY_ADC_INPUT_INV   0
#define RF_ANTENNA2_INV         0
#define RF_ANTENNA1_INV         0

#define RF_ANTENNA_DIR          P2DIR
#define RF_ANTENNA_OUT          P2OUT
#define RF_ANTENNA1             BIT0
#define RF_ANTENNA2             BIT1

#define BATTERY_ADC_INPUT_DIR   P2DIR
#define BATTERY_ADC_INPUT_OUT   P2OUT
#define BATTERY_ADC_INPUT       BIT2

#define LED_GREEN_DIR           P2DIR
#define LED_GREEN_OUT           P2OUT
#define LED_GREEN_PIN           BIT3

#define MD_DIR                  P2DIR
#define MD_IN                   P2IN
#define MD_PIN2                 BIT4
#define MD_INT                  P2IE
#define MD_INT_FLAG             P2IFG

#define OSC_SEL                 P2SEL
#define OSC_DIR                 P2DIR
#define OSC_OUT                 P2OUT
#define OSC_PIN                 BIT5

//PORT3
#define BATTERY_ADC_POWER_INV   0
#define LED_RED_INV             0
#define RF_GDO0_INV             0
#define RF_GDO2_INV             0
#define RF_SO_INV               0

#define LED_RED_DIR             P3DIR
#define LED_RED_OUT             P3OUT
#define LED_RED_PIN             BIT6

#define BATTERY_ADC_POWER_DIR   P3DIR
#define BATTERY_ADC_POWER_OUT   P3OUT
#define BATTERY_ADC_POWER       BIT7

//PORT4
#define UNUSED_PIN7_INV         0
#define UNUSED_PIN6_INV         0
#define UNUSED_PIN5_INV         0
#define UNUSED_PIN4_INV         0
#define UNUSED_PIN3_INV         0
#define LF_ENV_SIGNAL_INV       0
#define UNUSED_PIN1_INV         0
#define LF_STEPUP_CONV_INV      0

#define LF_STEPUP_CONV_DIR      P4DIR
#define LF_STEPUP_CONV_OUT      P4OUT
#define LF_STEPUP_CONV          BIT0

#define LF_ENV_SIGNAL_DIR       P4DIR
#define LF_ENV_SIGNAL_OUT       P4OUT
#define LF_ENV_SIGNAL           BIT2

#define BTN_INT_FLAG            P4IFG
#define BTN_INT                 P4IE
#define BTN_DIR                 P4DIR
#define BTN_IN                  P4IN
#define BTN_OUT                 P4OUT
#define BTN1_PIN                BIT7
#define BTN1_REN                P4REN

#ifdef __ENABLE_DEBUG__
#define DBG_INIT_MONITOR()           //P1DIR |= ( BIT6 )

#define DBG_SET_RECEIVE_PIN()       //P1DIR |= BIT5; P1OUT |= BIT5
#define DBG_RESET_RECEIVE_PIN()     //P1DIR |= BIT5; P1OUT &= ~BIT5

#define DBG_SET_SEND_PIN()          //P1DIR |= BIT5; P1OUT |= BIT5
#define DBG_RESET_SEND_PIN()        //P1DIR |= BIT5; P1OUT &= ~BIT5
#define DBG_TOGGLE_SEND_PIN()       //P1DIR |= BIT5; P1OUT ^= BIT5

#define DBG_TOGGLE_TIMER_PIN()      //P4DIR |= BIT7;P4OUT ^= BIT7
#define DBG_SET_TIMER_PIN()         //P1OUT |= BIT1
#define DBG_RESET_TIMER_PIN()       //P1OUT &= ~BIT1
#else
#define DBG_INIT_MONITOR()           //P4DIR |= ( BIT1 )
#define DBG_INIT_MONITOR_TIMER()     //P4DIR |= ( BIT7 )

#define DBG_SET_RECEIVE_PIN()       //P4DIR |= BIT5; P4OUT |= BIT5
#define DBG_RESET_RECEIVE_PIN()     //P4DIR |= BIT5; P4OUT &= ~BIT5

#define DBG_SET_SEND_PIN()          //P1OUT |= BIT1
#define DBG_RESET_SEND_PIN()        //P1OUT &= ~BIT1

#define DBG_TOGGLE_TIMER_PIN()      //P4DIR |= BIT4; P4OUT ^= BIT4
#define DBG_SET_TIMER_PIN()         //P1OUT |= BIT1
#define DBG_RESET_TIMER_PIN()       //P1OUT &= ~BIT1
#endif

#endif
