#include "defines.h"
#include "rf_spi_drv.h"
#include "timer_drv.h"
#include "general.h"

//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_Init()
//
//  DESCRIPTION:
//      This function intializes the SPI settings.
//
//-------------------------------------------------------------------------------------------------------

VOID RF_SPI_DRV_Init()
{
    // Init Port 1
    RF_CSN_DIR |=  RF_CSN;

    // Enable SPI Mode
    RF_SPI_DRV_Enable();

    UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                     // SMCLK
    UCB0BR0 |= 0x02;
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;

    // Disable  SPI Mode
    RF_SPI_DRV_Disable();
}

//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_Enable()
//
//  DESCRIPTION:
//      This function Enables the SPI Mode.
//
//-------------------------------------------------------------------------------------------------------
VOID RF_SPI_DRV_Enable()
{
    RF_SI_SEL |= RF_SI;

    RF_SO_DIR &= ~RF_SO;
    RF_SO_SEL |= RF_SO;
    RF_SCLK_SEL |= RF_SCLK;
}

//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_Disable()
//
//  DESCRIPTION:
//      This function Disables the SPI Mode.
//
//-------------------------------------------------------------------------------------------------------
VOID RF_SPI_DRV_Disable()
{
    RF_SI_DIR |= RF_SI;
    RF_SI_OUT |= RF_SI;
    RF_SI_SEL &= ~RF_SI;

    RF_SO_DIR &= ~RF_SO;
    RF_SO_OUT &= ~RF_SO;
    RF_SO_SEL &= ~RF_SO;

    RF_SCLK_DIR |= RF_SCLK;
    RF_SCLK_OUT |= RF_SCLK;
    RF_SCLK_SEL &= ~RF_SCLK;
}

//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_WriteByte(BYTE buff)
//
//  DESCRIPTION:
//      This function writes the given byte to TX Buffer.
//
//-------------------------------------------------------------------------------------------------------
VOID RF_SPI_DRV_WriteByte(BYTE byte)
{
    WORD counter;

    IFG2 &= ~UCB0RXIFG;                       // Clear flag set during last write
    UCB0TXBUF = byte;

    counter = RESET_TIMEOUT;
    while ( --counter && !(IFG2 & UCB0RXIFG) );      // USART0 TX buffer ready?
	Reset(counter);
}

//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_WriteBytes(BYTE buff)
//
//  DESCRIPTION:
//      This function writes n bytes to TX Buffer.
//
//-------------------------------------------------------------------------------------------------------
VOID RF_SPI_DRV_WriteBytes(BYTE *buff, BYTE count)
{
    BYTE idx;
    WORD counter;
    for(idx=0; idx<count; idx++)
    {
        counter = RESET_TIMEOUT;
    	while ( --counter && !(IFG2 & UCB0TXIFG) );  // USART0 TX buffer ready?
	    Reset(counter);

        UCB0TXBUF = buff[idx];
    }

    counter = RESET_TIMEOUT;
    while ( --counter && !(IFG2 & UCB0TXIFG) );      // USART0 TX buffer ready?
	Reset(counter);
}


//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_ReadByte(BYTE *buff)
//
//  DESCRIPTION:
//      This function reads a byte from RX Buffer.
//
//-------------------------------------------------------------------------------------------------------
VOID RF_SPI_DRV_ReadByte(BYTE *byte)
{
    WORD counter;
    counter = RESET_TIMEOUT;
    while ( --counter && !(IFG2 & UCB0TXIFG) );  // USART0 TX buffer ready?
	Reset(counter);

    UCB0TXBUF = 0xff;

    counter = RESET_TIMEOUT;
    while ( --counter && !(IFG2 & UCB0TXIFG) );  // USART0 TX buffer ready?
	Reset(counter);

    *byte = UCB0RXBUF;

}

//-------------------------------------------------------------------------------------------------------
//  VOID RF_SPI_DRV_ReadBytes(BYTE *buff)
//
//  DESCRIPTION:
//      This function reads n bytes from RX Buffer.
//
//-------------------------------------------------------------------------------------------------------
VOID RF_SPI_DRV_ReadBytes(BYTE *buff, BYTE count)
{
    BYTE idx;
    for(idx=0; idx<count; idx++)
    {
        buff[idx] = 0;
        RF_SPI_DRV_ReadByte( &buff[idx] );
    }
}
