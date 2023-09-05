//******************************************************************************
//  MSP430F20xx - I2C Master Transmitter and Receiver via CPU Software (GPIO)
//
//  Description: This code library configures the MSP430 as an I2C master device
//  capable of transmitting and receiving bytes using GPIO pins.  Specific I/O
//  pins can be selected in the corresponding header file.  By default, the same
//  pins that are used by the USI module are selected.
//
//                  Master                   
//                  MSP430          
//             -----------------          
//         /|\|              XIN|-   
//          | |                 |     
//          --|RST          XOUT|-    
//            |                 |        
//            |                 |        
//            |                 |       
//            |         SDA/P1.0|-------> [I2C SLAVE SDA]
//            |         SCL/P1.1|-------> [I2C SLAVE SCL]
//
//  Note: Internal pull-ups are NOT used for SDA & SCL [DISABLED]
//
//  R. Wu
//  Texas Instruments Inc.
//  March 2008
//
//******************************************************************************
#include "defines.h"
#include "i2c_drv.h"

void I2CMST_init(void)
{
    I2C_SEL &= ~(I2C_SCL | I2C_SDA);                    // Set GPIO function
    I2C_DIR |= (I2C_SCL | I2C_SDA);                     // Set output direction
    I2C_OUT &= ~(I2C_SCL | I2C_SDA);                    // SCL & SDA = 0, low when = outputs 
}

void I2CMST_readBlock(BYTE SlaveAddress, BYTE numBytes, void* RxData)
{
    BYTE* temp;
  
    temp = (BYTE *)RxData;                              // Initialize array pointer
    I2CMST_start();                                     // Send Start condition
    for (BYTE i = 0; i < numBytes; i++) {
        I2CMST_txByte((SlaveAddress << 1) | I2C_SDA);   // [ADDR] + R/W bit = 1
        *(temp) = I2CMST_rxByte();                      // Read 8 bits of data and send ack
        temp++;                                         // Increment pointer to next element
    }
    I2CMST_stop();                                      // Send Stop condition
}

void I2CMST_writeBlock(BYTE SlaveAddress, BYTE numBytes, void* TxData)
{        
    BYTE *temp;
   
    temp = (BYTE *)TxData;                              // Initialize array pointer
    I2CMST_start();                                     // Send Start condition
    I2CMST_txByte((SlaveAddress << 1) & ~I2C_SDA);      // [ADDR] + R/W bit = 0
    for (BYTE i = 0; i < numBytes; i++) {
        I2CMST_txByte(*(temp));                         // Send data and ack
        temp++;                                         // Increment pointer to next element
    }
    I2CMST_stop();                                      // Send Stop condition
}

void I2CMST_start(void)                                 // Set up start condition for I2C
{
    I2C_DIR |= I2C_SDA;                                 // Set to output, data low [SDA = 0]
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    I2C_DIR |= I2C_SCL;                                 // Set to output, data low [SCL = 0]
}

void I2CMST_txByte(BYTE data)
{
    BYTE bits, temp;
  
    temp = data;
    bits = 0x08;                                        // Load I2C bit counter
    while (bits != 0x00)                                // Loop until all bits are shifted
    {
        if (temp & BIT7)                                // Test data bit
            I2C_DIR &= ~I2C_SDA;                        // Set to input, SDA = 1 via pull-up
        else
          I2C_DIR |= I2C_SDA;                           // Set to output, data low [SDA = 0]
        I2C_DIR &= ~I2C_SCL;                            // Set to output, data low [SCL = 0]
        temp = (temp << 1);                             // Shift bits 1 place to the left
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        I2C_DIR |= I2C_SCL;                             // Set to output, data low [SCL = 0]
        bits = (bits - 1);                              // Loop until 8 bits are sent
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
    }
    I2C_DIR &= ~I2C_SDA;                                // Set to input, SDA = 1 via pull-up
    I2CMST_ack();                                       // Send acknowledge
}

void I2CMST_ack(void)                                   // Set up for I2C acknowledge
{
    I2C_DIR &= ~I2C_SCL;                                // Set to input, SCL = 1 via pull-up
    _NOP();                                             // delay to meet I2C spec
    _NOP();                                             //   "        "        "
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    I2C_DIR |= I2C_SCL;                                 // Set to output, data low [SCL = 0]
}
            
BYTE I2CMST_rxByte(void)                                // Read 8 bits of I2C data
{
    BYTE bits, temp = 0;                                // I2C bit counter

    bits = 0x08;                                        // Load I2C bit counter
    while (bits > 0)                                    // Loop until all bits are read
    {
        I2C_DIR &= ~I2C_SCL;                            // Set to input, SDL = 1 via pull-up
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        temp = (temp << 1);                             // Shift bits 1 place to the left
        if (I2C_IN & I2C_SDA)                           // Check digital input
            temp = (temp + 1);                          // If input is 'H' store a '1'
        I2C_DIR |= I2C_SCL;                             // Set to output, data low [SCL = 0]
        bits = (bits - 1);                              // Decrement I2C bit counter
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
        _NOP();                                         // Quick delay
    }
    I2CMST_ack();                                       // Send acknowledge
    return (temp);                                      // Return 8-bit data byte
}   

void I2CMST_stop(void)                                  // Send I2C stop command
{
    I2C_DIR |= I2C_SDA;                                 // Set to output, data low [SCA = 0]
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    I2C_DIR &= ~I2C_SCL;                                // Set to input, SCL = 1 via pull-up
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    _NOP();                                             // Quick delay
    I2C_DIR &= ~I2C_SDA;                                // Set to input, SDA = 1 via pull-up
}
