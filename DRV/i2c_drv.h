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
//            |         SDA/P2.7|-------> [I2C SLAVE SDA]
//            |         SCL/P2.6|-------> [I2C SLAVE SCL]
//
//  Note: Internal pull-ups are NOT used for SDA & SCL [DISABLED]
//
//  R. Wu
//  Texas Instruments Inc.
//  March 2008
//
//******************************************************************************

void I2CMST_init(void);

void I2CMST_readBlock(BYTE SlaveAddress, BYTE numBytes, void* RxData);

void I2CMST_writeBlock(BYTE SlaveAddress, BYTE numBytes, void* TxData);

void I2CMST_start(void);

void I2CMST_txByte(BYTE data);

void I2CMST_ack(void);

BYTE I2CMST_rxByte(void);

void I2CMST_stop(void);
