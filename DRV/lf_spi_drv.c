#include "defines.h"
#include "flash_app.h"
#include "profile.h"
#include "lf_spi_drv.h"
#include "timer_drv.h"
#include "general.h"

static VOID lfSpiEnable()
{
    LF_SDI_SEL |= LF_SDI;

    LF_SDO_DIR &= ~LF_SDO;
    LF_SDO_SEL |= LF_SDO;
    LF_SCLK_SEL |= LF_SCLK;
}

static VOID lfSpiDisable()
{
    LF_SDI_DIR |= LF_SDI;
    LF_SDI_OUT |= LF_SDI;
    LF_SDI_SEL &= ~LF_SDI;

    LF_SDO_DIR &= ~LF_SDO;
    LF_SDO_OUT &= ~LF_SDO;
    LF_SDO_SEL &= ~LF_SDO;

    LF_SCLK_DIR |= LF_SCLK;
    LF_SCLK_OUT |= LF_SCLK;
    LF_SCLK_SEL &= ~LF_SCLK;
}

static VOID lfSpiWriteByte(BYTE byte)
{
    WORD counter;

    IFG2 &= ~UCA0RXIFG;                       // Clear flag set during last write
    UCA0TXBUF = byte;

    counter = RESET_TIMEOUT;
    while ( --counter && !(IFG2 & UCA0RXIFG) );      // USART0 TX buffer ready?
	Reset(counter);
}

static BYTE lfSpiReadByte()
{
    WORD counter;
    BYTE data;

    counter = RESET_TIMEOUT;
    while ( --counter && !(IFG2 & UCA0TXIFG) );  // USART0 TX buffer ready?
	Reset(counter);

    UCA0TXBUF = 0x00;
    TIMER_CCR_Delay(2);
    data = UCA0RXBUF;
    
    return data;
}

static VOID lfSpiWrite(BYTE addr, BYTE value, BOOL isDirectCommand)
{
    // Enable SPI Mode
    lfSpiEnable();

    LF_CSN_DIR |= LF_CSN;
    LF_CSN_OUT |= LF_CSN;
    
    if( isDirectCommand )    
    {
        //Send the Flag 0 that is to write
        addr |= BIT7;
    
        //set the burst access False
        addr |= BIT6;
    }
    else
    {
        //Send the Flag 0 that is to write
        addr &= ~BIT7;
    
        //set the burst access False
        addr &= ~BIT6;
    }
    

    //Send the address
    lfSpiWriteByte(addr);

    if( !isDirectCommand )
    {
        //Send the Data
        lfSpiWriteByte(value);
    }

    LF_CSN_OUT &= ~LF_CSN;
    
    // Disable  SPI Mode
    lfSpiDisable();
}

static BYTE GetBoostSenstivity()
{
    BYTE boostdamp=0x2;
    if( ((settings.LFRegisterConfig >> 5 ) & 0x1) > 0 )
        boostdamp = 0x22;

    return boostdamp;   
}

BYTE LF_SPI_DRV_GetAntennaDamping()
{
    BYTE AntennaDamping = 0xC1;
    if( (( settings.LFRegisterConfig >> 3) & 0x3) > 0 )
        AntennaDamping = 0xD1;
    
    return AntennaDamping;
}

BYTE LF_SPI_DRV_GetAntenna_GainReduction()
{
      BYTE damping = settings.LFRegisterConfig  & 0x1F;
      damping <<= 1;
      
      return damping;
}

VOID LF_SPI_DRV_Initialize()
{
    // If LF RX is disabled no need to initialize the LF Chip.
    if( settings.LFRXDisabled )
        return;
      
    UCA0CTL0 = 0;
    UCA0CTL1 = 0;

    UCA0CTL0 |= (UCMSB + UCMST + UCSYNC); 
    UCA0CTL1 |= (UCSWRST + UCSSEL1); 

    UCA0BR0 = 0x10;

    UCA0CTL0 &= (~UCCKPH); 
    UCA0CTL0 &= (~UCCKPL); 

    UCA0CTL1 &= ~UCSWRST; 

    LF_POWER_DIR |= LF_POWER;
    LF_POWER_OUT |= LF_POWER;
    
    LF_SPI_DRV_DirectCommand(0x4);    
    LF_SPI_DRV_DirectCommand(0x00);   

    LF_SPI_DRV_WriteByte(0, 0x0e);
    LF_SPI_DRV_WriteByte(1, LF_SPI_DRV_GetAntennaDamping());  
    LF_SPI_DRV_WriteByte(2, GetBoostSenstivity());  
    LF_SPI_DRV_WriteByte(3, 0x67);
    LF_SPI_DRV_WriteByte(4, LF_SPI_DRV_GetAntenna_GainReduction());
    LF_SPI_DRV_WriteByte(5, 0x69);
    LF_SPI_DRV_WriteByte(6, 0x96);
    LF_SPI_DRV_WriteByte(7, 0xeb);
    LF_SPI_DRV_WriteByte(8, 0x00);
#ifdef __STAFF_TAG__      
    LF_SPI_DRV_WriteByte(17, flash_settings.Capacitor[0]);
    LF_SPI_DRV_WriteByte(18, flash_settings.Capacitor[1]);
#endif    
}


VOID LF_SPI_DRV_WriteByte(BYTE addr, BYTE value)
{
    lfSpiWrite(addr, value, FALSE);
}

VOID LF_SPI_DRV_DirectCommand(BYTE addr)
{
    lfSpiWrite(addr, 0, TRUE);
}

BYTE LF_SPI_DRV_ReadByte(BYTE addr)
{
    BYTE data = 0;
    
    // Enable SPI Mode
    lfSpiEnable();

    LF_CSN_DIR |= LF_CSN;
    LF_CSN_OUT |= LF_CSN;
    
    //Set the Flag 1 that is to read
    addr |= BIT6;

    //set the burst access False
    addr &= ~BIT7;

    //Read data
    data = lfSpiReadByte();
    
    LF_CSN_OUT &= ~LF_CSN;

    // Disable  SPI Mode
    lfSpiDisable();

    return data;
}

