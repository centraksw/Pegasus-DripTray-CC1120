#include "defines.h"
#include "ccxx2x_drv.h"
#include "rf_spi_drv.h"
#include "timer_drv.h"

#define CRC_OK              0x80
#define RSSI                0
#define LQI                 1
#define BYTES_IN_RXFIFO     0x7F

#define TOTAL_REGISTER                  58
#define TOTAL_CHANGED_REGISTER          14

// Register, 50 Kbps, 76 Kbps, 150 Kbps
RF_SETTINGS rfs[TOTAL_REGISTER] = {
    {CC112X_IOCFG3,		0x02,	0x02,	0x02},
    {CC112X_IOCFG2,		0x02,	0x02,	0x02},
    {CC112X_IOCFG1,		0xB0,	0xB0,	0xB0},
    {CC112X_IOCFG0,		0x41,	0x41,	0x41},
    {CC112X_SYNC3,		0xD3,	0xD3,	0xD3},
    {CC112X_SYNC2,		0x91,	0x91,	0x91},
    {CC112X_SYNC1,		0xD3,	0xD3,	0xD3},
    {CC112X_SYNC0,		0x91,	0X91,	0X91},
    {CC112X_SYNC_CFG1,	        0x08,	0x0B,	0x0B},
    {CC112X_SYNC_CFG0,	        0x0B,	0x17,	0x17},
    {CC112X_DEVIATION_M,	0x99,	0x04,	0x33},
    {CC112X_MODCFG_DEV_E,	0x0D,	0x06,	0x27},
    {CC112X_DCFILT_CFG,	        0x15,	0x1C,	0x04},
    {CC112X_PREAMBLE_CFG1,	0x18,	0x18,	0x18},
    {CC112X_FREQ_IF_CFG,	0x3A,	0x40,	0x00},
    {CC112X_IQIC,		0x00,	0x46,	0x00},
    {CC112X_CHAN_BW,	        0x02,	0x01,	0x01},
    {CC112X_MDMCFG0,	        0x05,	0x05,	0x05},
    {CC112X_SYMBOL_RATE2,	0x99,	0xA3,	0xA3},
    {CC112X_SYMBOL_RATE1,	0x99,	0xA9,	0x33},
    {CC112X_SYMBOL_RATE0,	0x99,	0x2A,	0x33},
    {CC112X_AGC_REF,	        0x3C,	0x3C,	0x3C},
    {CC112X_AGC_CS_THR,	        0xEF,	0xEF,	0xEC},
    {CC112X_AGC_GAIN_ADJUST,    0x00,	0x00,	0x00},
    {CC112X_AGC_CFG3,	        0x91,	0x91,	0x83},
    {CC112X_AGC_CFG2,	        0x20,	0x20,	0x60},
    {CC112X_AGC_CFG1,	        0xA9,	0xA9,	0xA9},
    {CC112X_AGC_CFG0,	        0xC0,	0xCF,	0xC0},
    {CC112X_FIFO_CFG,	        0x7E,	0x7E,	0x7E},
    {CC112X_SETTLING_CFG,	0x03,	0x03,	0x03},
    {CC112X_FS_CFG,		0x12,	0x12,	0x12},
    {CC112X_PKT_CFG0,	        0x20,	0x20,	0x20},
    {CC112X_PA_CFG2,	        0x62,	0x62,	0x62},
    {CC112X_PA_CFG0,	        0x79,	0x42,	0x02},
    {CC112X_PKT_LEN,	        0xFF,	0xFF,	0xFF},
    {CC112X_IF_MIX_CFG,	        0x00,	0x00,	0x00},
    {CC112X_TOC_CFG,	        0x0A,	0x0B,	0x0A},
    {CC112X_FREQOFF_CFG,	0x20,	0x20, 	0x20},
    {CC112X_FREQ2,		0x6C,	0x72, 	0x72},
    {CC112X_FREQ1,		0x36,	0x40, 	0x60},
    {CC112X_FREQ0,		0x66,	0x00, 	0x00},
    {CC112X_FS_DIG1,	        0x00,	0x00, 	0x00},
    {CC112X_FS_DIG0,	        0x5F,	0x5F, 	0x5F},
    {CC112X_FS_CAL1,	        0x40,	0x40,	0x40},
    {CC112X_FS_CAL0,	        0x0E,	0x0E,	0x0E},
    {CC112X_FS_CHP,		0x28,	0x29,	0x28},
    {CC112X_FS_DIVTWO,	        0x03,	0x03,	0x03},
    {CC112X_FS_DSM0,	        0x33,	0x33,	0x33},
    {CC112X_FS_DVC0,	        0x17,	0x17,	0x17},
    {CC112X_FS_PFD,		0x50,	0x50,	0x50},
    {CC112X_FS_PRE,		0x6E,	0x6E,	0x6E},
    {CC112X_FS_REG_DIV_CML,	0x14,	0x14,	0x14},
    {CC112X_FS_SPARE,	        0xAC,	0xAC,	0xAC},
    {CC112X_FS_VCO4,	        0x14,	0x0E,	0x14},
    {CC112X_FS_VCO1,	        0x00,	0xBC,	0x00},
    {CC112X_FS_VCO0,	        0xB4,	0xB4,	0xB4},
    {CC112X_XOSC5,		0x0E,	0x0E,	0x0E},
    {CC112X_XOSC1,		0x03,	0x03,	0x03}
};

RF_150_50_SETTINGS Rf_150_50[TOTAL_CHANGED_REGISTER] = {
    {CC112X_SYNC_CFG1,	        0x08,	0x0B},
    {CC112X_SYNC_CFG0,	        0x0B,	0x17},
    {CC112X_DEVIATION_M,	0x99,	0x33},
    {CC112X_MODCFG_DEV_E,	0x0D,	0x27},
    {CC112X_DCFILT_CFG,	        0x15,	0x04},
    {CC112X_FREQ_IF_CFG,	0x3A,	0x00},
    {CC112X_CHAN_BW,	        0x02,	0x01},
    {CC112X_SYMBOL_RATE2,	0x99,	0xA3},
    {CC112X_SYMBOL_RATE1,	0x99,	0x33},
    {CC112X_SYMBOL_RATE0,	0x99,	0x33},
    {CC112X_AGC_CS_THR,	        0xEF,	0xEC},
    {CC112X_AGC_CFG3,	        0x91,	0x83},
    {CC112X_AGC_CFG2,	        0x20,	0x60},
    {CC112X_PA_CFG0,	        0x79,	0x02},
};
// Set CSN Low
static VOID Set_CSn_Low()
{
    RF_CSN_OUT &= ~RF_CSN;
}

static VOID Set_CSn_High()
{
    RF_CSN_OUT |= RF_CSN;
}

static BOOL Is_GDO0_High()
{
    RF_GDO0_DIR &= ~RF_GDO0;

    return (RF_GDO0_IN & RF_GDO0);
}

static BOOL Is_GDO2_High()
{
    return (RF_GDO2_IN & RF_GDO2);
}

//Used to make Wait until the SO pin from CCxxx0 goes low before transfer
static VOID SO_HIGH()
{
    WORD counter=0;
    RF_SO_DIR &= (~RF_SO);
    RF_SO_SEL &= (~RF_SO);

    counter = RESET_TIMEOUT;
    while( --counter && (RF_SO_IN & RF_SO));
    Reset(counter);

    RF_SO_SEL |= RF_SO;
    RF_SO_DIR |= RF_SO;
}

void CC_DRV_WriteRegisters(BYTE Bitrate)
{
    BYTE extAddr;
    BYTE regAddr;
    BYTE idx;

    RF_SPI_DRV_Enable();

    //Set CSN as LOW
    Set_CSn_Low();

    //Wait until the SO pin from CC112X goes low before transfer
    SO_HIGH();

    for(idx=0; idx<TOTAL_REGISTER; idx++)
    {
	extAddr = (BYTE)(rfs[idx].Register >> 8);
	regAddr = (BYTE)(rfs[idx].Register & 0x00FF);

	/* send extended address byte with access type bits set */
	if( extAddr )
	{
	    RF_SPI_DRV_WriteByte((RADIO_WRITE_ACCESS | extAddr));
	    RF_SPI_DRV_WriteByte(regAddr);
	}
	else
	{
	    RF_SPI_DRV_WriteByte((RADIO_WRITE_ACCESS | regAddr));
	}
    switch(Bitrate)
    {
        case BRATE_50_KBPS:
	    	RF_SPI_DRV_WriteByte(rfs[idx]._50kbps);
            break;
        case BRATE_76_KBPS:
	    RF_SPI_DRV_WriteByte(rfs[idx]._76kbps);
            break;
        case BRATE_150_KBPS:
	    RF_SPI_DRV_WriteByte(rfs[idx]._150kbps);
            break;
    }

    }

    //Set CSN as high
    Set_CSn_High();

    RF_SPI_DRV_Disable();
}

VOID CC_DRV_PowerupReset(BYTE RF_BitRate)
{
    RF_RESET_DIR |=  RF_RESET;
    RF_RESET_OUT &= ~RF_RESET;

    RF_SI_DIR |= RF_SI;
    RF_SI_OUT &= ~RF_SI;

    RF_SCLK_DIR |= RF_SCLK;
    RF_SCLK_OUT |= RF_SCLK;

    RF_RESET_DIR |=  RF_RESET;
    RF_RESET_OUT |=  RF_RESET;

    Set_CSn_High();
    TIMER_CCR_Delay(3);
    Set_CSn_Low();
    TIMER_CCR_Delay(3);
    Set_CSn_High();
    TIMER_CCR_Delay(3);

    RF_SPI_DRV_Enable();
    Set_CSn_Low();
    SO_HIGH();
    RF_SPI_DRV_WriteByte(CC112X_SRES);
    SO_HIGH();
    Set_CSn_High();
    RF_SPI_DRV_Disable();

    CC_DRV_WriteRegisters(RF_BitRate);
}

VOID CC_DRV_Strobe(BYTE strobe)
{
    // Enable SPI Mode
    RF_SPI_DRV_Enable();

    // Set CSN as LOW
    Set_CSn_Low();

    // Wait until the SO pin from CCxxx0 goes low before transfer
    SO_HIGH();

    // Send the strobe
    RF_SPI_DRV_WriteByte(strobe);

    // Set CSN as high
    Set_CSn_High();

    // Disable  SPI Mode
    RF_SPI_DRV_Disable();
}

VOID CC_DRV_WriteReg(WORD addr, BYTE value)
{
    BYTE tempExt  = (BYTE)(addr>>8);
    BYTE tempAddr = (BYTE)(addr & 0x00FF);

    if(!tempExt)
    {
        CC_DRV_8bitReadWriteReg((RADIO_WRITE_ACCESS),tempAddr, &value,1);
    }
    else
    {
        CC_DRV_16bitReadWriteReg((RADIO_WRITE_ACCESS),tempExt,tempAddr, &value,1);
    }
}

// Write in chipcon Registry
VOID CC_DRV_Switch_Bitrate(BYTE Bitrate)
{
    BYTE regAddr;
    BYTE idx;

    RF_SPI_DRV_Enable();

    //Set CSN as LOW
    Set_CSn_Low();

    //Wait until the SO pin from CC112X goes low before transfer
    SO_HIGH();

    for(idx=0; idx< TOTAL_CHANGED_REGISTER; idx++)
    {
	regAddr = (BYTE)(Rf_150_50[idx].Register & 0x00FF);

        RF_SPI_DRV_WriteByte((RADIO_WRITE_ACCESS | regAddr));

        switch(Bitrate)
        {
        case BRATE_50_KBPS:
            RF_SPI_DRV_WriteByte(Rf_150_50[idx]._50kbps);
            break;

        case BRATE_150_KBPS:
            RF_SPI_DRV_WriteByte(Rf_150_50[idx]._150kbps);
            break;
        }
    }
    //Set CSN as high
    Set_CSn_High();

    RF_SPI_DRV_Disable();
}

BYTE CC_DRV_ReadStatus(WORD addr)
{
    BYTE tempExt  = (BYTE)(addr>>8);
    BYTE tempAddr = (BYTE)(addr & 0x00FF);

    BYTE pData = 0;

    if(!tempExt)
    {
        CC_DRV_8bitReadWriteReg((RADIO_READ_ACCESS),tempAddr,&pData,1);
    }
    else if (tempExt == 0x2F)
    {
        CC_DRV_16bitReadWriteReg((RADIO_READ_ACCESS),tempExt,tempAddr,&pData,1);
    }
    return pData;
}

BYTE CC_DRV_ReadReg(WORD addr)
{
    BYTE tempExt  = (BYTE)(addr>>8);
    BYTE  tempAddr = (BYTE)(addr & 0x00FF);

    BYTE pData = 0;

    if(!tempExt)
    {
        CC_DRV_8bitReadWriteReg((RADIO_READ_ACCESS),tempAddr,&pData,1);
    }
    else if (tempExt == 0x2F)
    {
        CC_DRV_16bitReadWriteReg((RADIO_READ_ACCESS),tempExt,tempAddr,&pData,1);
    }

    return pData;
}

VOID CC_DRV_WriteBurstReg(BYTE addr, BYTE *buffer, BYTE count)
{
    BYTE tempExt  = (BYTE)(addr>>8);
    BYTE tempAddr = (BYTE)(addr & 0x00FF);

    if(!tempExt)
    {
        CC_DRV_8bitReadWriteReg((RADIO_BURST_ACCESS|RADIO_WRITE_ACCESS),tempAddr,buffer,count);
    }
    else
    {
        CC_DRV_16bitReadWriteReg((RADIO_BURST_ACCESS|RADIO_WRITE_ACCESS),tempExt,tempAddr,buffer,count);
    }
}

void CC_DRV_8bitReadWriteReg(BYTE accessType, BYTE addrByte, BYTE *pData, WORD len)
{
    BYTE addr = 0;

    // Enable SPI Mode
    RF_SPI_DRV_Enable();

    //Set CSN as LOW
    Set_CSn_Low();

    //Wait until the SO pin from CC112X goes low before transfer
    SO_HIGH();

    /* send register address byte */
    RF_SPI_DRV_WriteByte((accessType|addrByte));

    addr = accessType|addrByte;

    if(addr&RADIO_READ_ACCESS)
    {
        if(addr&RADIO_BURST_ACCESS)
        {
            RF_SPI_DRV_ReadBytes(pData, len);
        }
        else
        {
            RF_SPI_DRV_ReadByte(pData);
        }
    }
    else
    {
        if(addr&RADIO_BURST_ACCESS)
        {
            RF_SPI_DRV_WriteBytes(pData, len);
        }
        else
        {
            RF_SPI_DRV_WriteByte(*pData);
        }
    }
    //Set CSN as high
    Set_CSn_High();

    RF_SPI_DRV_Disable();
}

void CC_DRV_16bitReadWriteReg(BYTE accessType, BYTE extAddr, BYTE regAddr, BYTE *pData, BYTE len)
{
    BYTE addr = 0;

    RF_SPI_DRV_Enable();

    //Set CSN as LOW
    Set_CSn_Low();

    //Wait until the SO pin from CC112X goes low before transfer
    SO_HIGH();

    /* send extended address byte with access type bits set */
    RF_SPI_DRV_WriteByte((accessType|extAddr));

    RF_SPI_DRV_WriteByte(regAddr);

    addr = accessType|extAddr;

    if(addr&RADIO_READ_ACCESS)
    {
        if(addr&RADIO_BURST_ACCESS)
        {
            RF_SPI_DRV_ReadBytes(pData, len);
        }
        else
        {
            RF_SPI_DRV_ReadByte(pData);
        }
    }
    else
    {
        if(addr&RADIO_BURST_ACCESS)
        {
            RF_SPI_DRV_WriteBytes(pData, len);
        }
        else
        {
            RF_SPI_DRV_WriteByte(*pData);
        }
    }
    //Set CSN as high
    Set_CSn_High();

    RF_SPI_DRV_Disable();
}

VOID CC_DRV_ReadBurstReg(WORD addr, BYTE *buffer, BYTE count)
{
    BYTE tempExt  = (BYTE)(addr>>8);
    BYTE  tempAddr = (BYTE)(addr & 0x00FF);

    if(!tempExt)
    {
        CC_DRV_8bitReadWriteReg((RADIO_BURST_ACCESS|RADIO_READ_ACCESS),tempAddr,buffer,count);
    }
    else if (tempExt == 0x2F)
    {
        CC_DRV_16bitReadWriteReg((RADIO_BURST_ACCESS|RADIO_READ_ACCESS),tempExt,tempAddr,buffer,count);
    }
}

VOID CC_DRV_SendPacket(BYTE *txBuf)
{
    WORD idx;
    BYTE size = txBuf[0] + 1;
    RF_GDO2_DIR &= ~RF_GDO2;

    // Disable RX and goto IDLE
    CC_DRV_Strobe(CC112X_SIDLE);

    CC_DRV_Strobe(CC112X_SFRX);

    CC_DRV_WriteBurstReg(CC112X_BURST_TXFIFO, txBuf, size);

    CC_DRV_Strobe(CC112X_STX);

    for(idx = 0; idx < 5000; idx++)
    {
        if ( !Is_GDO2_High())  break;
    }

    TIMER_CCR_Delay(TAG_FLYOVER_TIME);
}

BYTE CC_DRV_Receive(BYTE *rxBuf, BYTE size)
{
    WORD rxLen, pktLen;

    // Check is the rxBuffer having enough space to store RX Packet and RSSI
    if( size > MAX_PKT_LEN )
        return FALSE;

    // Check is there any data available in RX FIFO
    if( Is_GDO0_High() )
        return FALSE;

    DBG_RESET_RECEIVE_PIN();
    // Disable RX and goto IDLE
    CC_DRV_Strobe(CC112X_SIDLE);

    // Read length byte
    pktLen = CC_DRV_ReadReg(CC112X_SINGLE_RXFIFO);

    DBG_SET_RECEIVE_PIN();

    if ( (pktLen + 2) <= MAX_PKT_LEN )
    {
        // Read no. of bytes in RX FIFO
        rxLen = (CC_DRV_ReadStatus(CC112X_NUM_RXBYTES) & BYTES_IN_RXFIFO);
        if (rxLen >= (pktLen + 2))
        {
            // Read data + (2 byte status RSSI, LQI) from RX FIFO and store in rxBuffer
            CC_DRV_ReadBurstReg(CC112X_BURST_RXFIFO, rxBuf, pktLen + 2);
            // MSB of LQI is the CRC_OK bit
            if (rxBuf[pktLen + LQI] & CRC_OK )
                return pktLen;
        }
    }
    // Flush RX FIFO
    CC_DRV_Strobe(CCxxx0_SFRX);

    return 0;
}

#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2

void manualCalibration()
{
    BYTE original_fs_cal2;
    BYTE  calResults_for_vcdac_start_high[3];
    BYTE calResults_for_vcdac_start_mid[3];
    BYTE marcstate;
    BYTE writeByte;

    // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    CC_DRV_WriteReg(CC112X_FS_VCO2, writeByte);

    // 2) Start with high VCDAC (original VCDAC_START + 2):
    original_fs_cal2 = CC_DRV_ReadReg(CC112X_FS_CAL2);
    writeByte = original_fs_cal2 + VCDAC_START_OFFSET;
    CC_DRV_WriteReg(CC112X_FS_CAL2, writeByte);

    // 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
    CC_DRV_Strobe(CC112X_SCAL);

    do
    {
        marcstate = CC_DRV_ReadReg(CC112X_MARCSTATE);
    } while (marcstate != 0x41);

    // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
    calResults_for_vcdac_start_high[FS_VCO2_INDEX] = CC_DRV_ReadReg(CC112X_FS_VCO2);
    calResults_for_vcdac_start_high[FS_VCO4_INDEX] = CC_DRV_ReadReg(CC112X_FS_VCO4);
    calResults_for_vcdac_start_high[FS_CHP_INDEX] = CC_DRV_ReadReg(CC112X_FS_CHP);

    // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    CC_DRV_WriteReg(CC112X_FS_VCO2, writeByte);

    // 6) Continue with mid VCDAC (original VCDAC_START):
    writeByte = original_fs_cal2;
    CC_DRV_WriteReg(CC112X_FS_CAL2, writeByte);

    // 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
    CC_DRV_Strobe(CC112X_SCAL);

    do
    {
        marcstate = CC_DRV_ReadReg(CC112X_MARCSTATE);
    } while (marcstate != 0x41);

    // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
    calResults_for_vcdac_start_mid[FS_VCO2_INDEX] = CC_DRV_ReadReg(CC112X_FS_VCO2);
    calResults_for_vcdac_start_mid[FS_VCO4_INDEX] = CC_DRV_ReadReg(CC112X_FS_VCO4);
    calResults_for_vcdac_start_mid[FS_CHP_INDEX] = CC_DRV_ReadReg(CC112X_FS_CHP);

    // 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
    if (calResults_for_vcdac_start_high[FS_VCO2_INDEX] > calResults_for_vcdac_start_mid[FS_VCO2_INDEX])
    {
        writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
        CC_DRV_WriteReg(CC112X_FS_VCO2, writeByte);
        writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
        CC_DRV_WriteReg(CC112X_FS_VCO4, writeByte);
        writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
        CC_DRV_WriteReg(CC112X_FS_CHP, writeByte);
    }
    else
    {
        writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
        CC_DRV_WriteReg(CC112X_FS_VCO2, writeByte);
        writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
        CC_DRV_WriteReg(CC112X_FS_VCO4, writeByte);
        writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
        CC_DRV_WriteReg(CC112X_FS_CHP, writeByte);
    }
}

// Set the Chipcon RF Parameters
VOID CC_DRV_SetRFParameters(BYTE Mdmcfg1, BYTE Freq1, BYTE Freq0)
{
    rfs[3]._76kbps= Freq1;		//CCxxx0_FREQ1
    rfs[4]._76kbps= Freq0;		//CCxxx0_FREQ0
    rfs[8]._76kbps= Mdmcfg1;		//CCxxx0_MDMCFG1

    rfs[3]._50kbps= Freq1;		        //CCxxx0_FREQ1
    rfs[4]._50kbps= Freq0;		        //CCxxx0_FREQ0
    rfs[8]._50kbps= Mdmcfg1;		//CCxxx0_MDMCFG1

    rfs[3]._150kbps= Freq1;		//CCxxx0_FREQ1
    rfs[4]._150kbps= Freq0;		//CCxxx0_FREQ0
    rfs[8]._150kbps= Mdmcfg1;		//CCxxx0_MDMCFG1
}

VOID setPreamlebyte(BYTE freqIdx, BOOL blnSpecialBeacon, BOOL bln30System )
{
    if(blnSpecialBeacon)
    {
        CC_DRV_WriteReg(CC112X_IOCFG0,  0x01);
        Set16BytePreamble(0x2C);
    }
    else
    {
        CC_DRV_WriteReg(CC112X_IOCFG0,  0x41);
        Set16BytePreamble(0x18);
    }
}

VOID Set16BytePreamble(BYTE value)
{
    CC_DRV_WriteReg(CC112X_PREAMBLE_CFG1,  value);
}