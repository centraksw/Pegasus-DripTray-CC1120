#include <string.h>
#include "defines.h"
#include "rf.h"
#include "ccxx2x_drv.h"
#include "timer_drv.h"
#include "profile.h"
#include "flash_app.h"

static const FREQUENCY data_freq[15] = {
  {0x72, 0x40, 0x00}, // 914
  {0x72, 0x60, 0x00}, // 915
  {0x72, 0x80, 0x00}, // 916
  {0x72, 0xA0, 0x00}, // 917
  {0x72, 0xC0, 0x00}, // 918
  {0x72, 0xE0, 0x00}, // 919
  {0x72, 0xA0, 0x00}, // 917 (Spectralink)
  {0x72, 0xC0, 0x00}, // 918 (Spectralink)
  {0x6C, 0xBC, 0xCD}, // 869.9 (EU)
  {0x6C, 0xBC, 0xCD}, // 869.9 (EU)
  {0x73, 0xC0, 0x00}, // 926 (AU)
  {0x6C, 0x49, 0x99}, // 866.3 (IN)
  {0x6C, 0x56, 0x66}, // 866.7 (IN)
  {0x6C, 0x8B, 0x33}, //868.35 (CN)
  {0x73, 0x70, 0x00}  //923.5  (MO)
};

static const FREQUENCY beacon_freq[15] = {
  {0x71, 0x00, 0x00}, // 904
  {0x71, 0x20, 0x00}, // 955
  {0x71, 0x40, 0x00}, // 906
  {0x71, 0x60, 0x00}, // 907
  {0x71, 0x80, 0x00}, // 908
  {0x71, 0xA0, 0x00}, // 909
  {0x72, 0x00, 0x00}, // 912 (Spectralink)
  {0x72, 0x60, 0x00}, // 915 (Spectralink)
  {0x6C, 0x9C, 0xCD}, // 868.9 (EU)
  {0x6C, 0x8C, 0xCD}, // 868.4 (EU)
  {0x73, 0x40, 0x00}, // 922 (AU)
  {0x6C, 0x29, 0x99}, // 865.3 (IN)
  {0x6C, 0x36, 0x66}, // 865.7 (IN)
  {0x6C, 0x88, 0x00}, //868.25 (CN)
  {0x73, 0x30, 0x00}  //921.5  (MO)
};

BYTE buf[MAX_PKT_LEN];
static BYTE freqIdx = 1;

VOID  RF_Init(BYTE Bitrate)
{
    // Reset RF chip
    CC_DRV_PowerupReset(Bitrate);

     CC_DRV_Strobe(CC112X_SIDLE);

    // Flush RX FIFO
    CC_DRV_Strobe(CC112X_SFRX);

    RF_TurnOff();
}

VOID RF_SetFrequencyParams(BYTE fIdx, BYTE* BeaconChannel, BYTE* DataChannel)
{
    if(fIdx < 1 || fIdx > MAX_FREQUENCY ) fIdx = 1;
    fIdx -= 1;
    freqIdx = fIdx;
    *BeaconChannel = 1;
    *DataChannel = 2;
}

// Initialize Antenna Settings
VOID RF_Init_Antenna()
{
    RF_ANTENNA_OUT |= RF_ANTENNA1;
    RF_ANTENNA_DIR |= (RF_ANTENNA1 | RF_ANTENNA2);
}

VOID RF_SetAntenna(BYTE Antenna)
{
    RF_ANTENNA_OUT &= ~(RF_ANTENNA1 | RF_ANTENNA2);
    RF_ANTENNA_OUT |= Antenna;
}

VOID RF_SetTXPower(BYTE power)
{
    CC_DRV_WriteReg(CC112X_PA_CFG2, power);
}

VOID RF_SetChannel(BYTE Channel)
{
    BYTE Freq2, Freq1, Freq0;
    switch(Channel)
    {
    case 1:
        Freq2 = beacon_freq[freqIdx].Freq2;
        Freq1 = beacon_freq[freqIdx].Freq1;
        Freq0 = beacon_freq[freqIdx].Freq0;
        break;
    case 2:
        Freq2 = data_freq[freqIdx].Freq2;
        Freq1 = data_freq[freqIdx].Freq1;
        Freq0 = data_freq[freqIdx].Freq0;
        break;
    }
    CC_DRV_Strobe(CC112X_SIDLE);

    CC_DRV_WriteReg(CC112X_FREQ2, Freq2);
    CC_DRV_WriteReg(CC112X_FREQ1, Freq1);
    CC_DRV_WriteReg(CC112X_FREQ0, Freq0);

    CC_DRV_Strobe(CC112X_SCAL);
    TIMER_DelayUS(300);
}

VOID RF_Switch_Bitrate(BYTE Bitrate)
{
    CC_DRV_Switch_Bitrate(Bitrate);
    RF_TurnOff();
}

BYTE RF_ReceiveBeaconPacket(BYTE *rxBuf, BYTE rxBufLen, BYTE FT, BYTE HT )
{
    BYTE len;
    memset(rxBuf, 0, rxBufLen);
    DBG_SET_RECEIVE_PIN();
    len = RF_Receive(rxBuf, rxBufLen, FT, HT);
    DBG_RESET_RECEIVE_PIN();
    return len;
}

BYTE RF_ReceivePacket(BYTE *rxBuf, BYTE rxBufLen, BYTE retry)
{
    BYTE len;
    memset(rxBuf, 0, rxBufLen);
    DBG_SET_RECEIVE_PIN();
    len = RF_Receive(rxBuf, rxBufLen, retry, retry);
    DBG_RESET_RECEIVE_PIN();
    return len;
}
VOID RF_SendPacket(BYTE *txBuf)
{
    DBG_SET_RECEIVE_PIN();
    CC_DRV_SendPacket( txBuf );
    DBG_RESET_RECEIVE_PIN();
}

WORD radio_state;
BYTE RF_Receive(BYTE* rxBuf, BYTE size, BYTE FT, BYTE HT)
{
   //DBG_SET_RECEIVE_PIN();

    WORD timeout = 0;
    BYTE pktLen, cntr=0, rxLen;
    radio_state = CC_DRV_ReadStatus(CC112X_MARCSTATE);
    if((radio_state & 0x0F) != 0x0D)
        CC_DRV_Strobe(CCxxx0_SRX);
    while( 1 )
    {
        pktLen = CC_DRV_Receive( rxBuf, size );

        if( pktLen > 0)
            break;

        if( ++timeout == (30 + FT * 3) )
            break;

        rxLen = (CC_DRV_ReadStatus(CCxxx0_RXBYTES) & 0x7F);
        if( rxLen <= 0 )
        {
            if( ++cntr > (20 + HT * 2) )
                break;
        }
    }
   // DBG_RESET_RECEIVE_PIN();
    RF_TurnOff();
    return pktLen;
}

////////////////////////////////////////////////////////////////////////////////
//
// Beacon Packet Structure 3 Bytes
//
// AAAA AAAA  BXXX XXXX CCCC DDDD
//
// A = Star Id
// B = Even / Odd Beacon
// C = Broadcast Command
// D = Broadcast Data
// X = Free bits
//
////////////////////////////////////////////////////////////////////////////////

BOOL RF_ReceiveBeacon(BEACON* beacon, BYTE channel, BYTE FT, BYTE HT)
{
    BYTE* rxBuf = buf;
    BYTE pktLen;
    BOOL blnRes = FALSE;

    memset(rxBuf, 0, MAX_PKT_LEN);
    RF_SetChannel( channel );

    pktLen = RF_ReceiveBeaconPacket( rxBuf, MAX_PKT_LEN, FT, HT);


    if( pktLen == BEACON_PKT_LEN )
    {
        memcpy(&beacon->StarId, rxBuf+0, 2);
        beacon->StarId &= 0xFFF;

        beacon->Info = rxBuf[1];

        beacon->Command = rxBuf[2];

        beacon->rssi = rxBuf[4];

        blnRes = TRUE;
    }

    return blnRes;
}

VOID RF_SendPagingRequest(DWORD DeviceId, WORD LBIValue, BYTE Version, BYTE Idx)
{
    BYTE* txBuf = buf;

    txBuf[0] = PAGE_REQ_PKT_LEN_EX ;

    txBuf[1] = RF_HDR_PAGING_REQUEST_EX | ((Idx & 0xF) << 4);

    memcpy(txBuf+2, &DeviceId, 3);

    memcpy(txBuf+5, &LBIValue, 2);

    txBuf[7] = Version;

    RF_SendPacket( txBuf );
}

VOID RF_SendPagingRequest3x(DWORD DeviceId, BYTE Part, BYTE Version, BYTE Idx, BYTE MotionFlag, BYTE PsuedoSync )
{
    BYTE* txBuf = buf;
    WORD Value = 0;

    // Paging request size 10 bytes
    txBuf[0] = PAGE_REQ_PKT_LEN_3X ;

    // Paging request size value: 16
    txBuf[1] = RF_HDR_PAGING_REQUEST_3X;

    // Device Id
    memcpy(txBuf+2, &DeviceId, 4);

    //Tag type
    txBuf[6] = MONITOR_TYPE;

    //Tag type  CDBB BBAA AAAA AAAA
    // A - Version 10 bits
    // B - Paging Index 4 bits
    // C - Motion flag
    // D - Psuedo Sync or not
    Value = Version & 0x3FF;
    Value |= ((Idx & 0xF) << 10);
    Value |= ((PsuedoSync & 0x1) << 15);
    memcpy(txBuf+7, &Value, 2);

    // XXXX XXXX XXXX XXXA
    // A - Part info
    // X - Reserved bits
    Value = 0;
    Value |= Part & 0x1;
    memcpy(txBuf+9, &Value, 2);
    txBuf[11] = flash_settings.HardwareVersion;

    RF_SendPacket( txBuf );
}

////////////////////////////////////////////////////////////////////////////////
//
// Page Response Packet Structure 10 Bytes
//
// AAAA BBBB CCCC CCCC CCCC CCCC CCCC CCCC DDDD DDDD DDDD DDDD
// EEEE EEEE FFFF FFFF GGGG GGGG HHHH HHHH IIII IIII IIII IIII
//
// A = Free Bits
// B = Packet Header
// C = Tag / Monitor Id
// D = StarId
// E = Current Slot
// F = Milliseconds remaining in current slot
// G = Profile 1
// H = Profile 2
// I = Extended Profile

// Profile1 : DCCC XXBA (MONITOR)
// A: IR Profile  [0 : 1.5 secs; 1 : 3 secs]
// B: Master / Slave Monitor
// X: Free Bits
// C: Power Level
// D: Upgrade Bit.

// Profile2 : CCBA AAAA (MONITOR)
// A: Monitor Profile
// B: Masking
// C: Power Injection

// Profile1 : BXXX XXXA (TAG)
// A: IR Profile  [0 : 1.5 secs; 1 : 3 secs]
// X: Free Bits
// B: Upgrade Bit.

// Profile2 : EDDC CBBA (TAG)
// A: Tag Profile
// B: IR Reporting Time
// C: NO IR Reporting Time
// D: Motion Sensor Scan logic
// E: Tag Type [Asset / Patient]
//
// Extended Profile: AAAA BBBB BBBB BBBB (MONITOR)
// A: Beacon Count [0 - 2]
// B: RoomId
////////////////////////////////////////////////////////////////////////////////
BOOL RF_ReceivePagingResponse(DWORD DeviceId, PAGE_RESPONSE* response)
{
    BYTE* rxBuf = buf;
    BYTE pktLen;
    BOOL blnRes = FALSE;

    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket( rxBuf, MAX_PKT_LEN, 0 );
    if( pktLen == PAGE_RES_PKT_LEN_EX && (rxBuf[0] & 0xF) == RF_HDR_PAGING_RESPONSE_EX )
    {
        memset(response, 0, sizeof(PAGE_RESPONSE));
        memcpy(&response->DeviceId, rxBuf+1, 3);
        if( response->DeviceId == DeviceId )
        {
            memcpy(&response->StarId, rxBuf+4, 2);
            response->Slot = rxBuf[6];
            response->Offset = rxBuf[7];
            response->Profile1 = rxBuf[8];
            response->Profile2 = rxBuf[9];
            memcpy(&response->Profile3, rxBuf+10, 2);
            blnRes = TRUE;
        }
    }
    return blnRes;
}

BOOL RF_ReceivePagingResponse3x(DWORD DeviceId, PAGE_RESPONSE* response)
{
    BYTE* rxBuf = buf;
    BYTE pktLen;
    BOOL blnRes = FALSE;

    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket( rxBuf, MAX_PKT_LEN, 12 );
    if( pktLen == PAGE_RES_PKT_LEN_3X && (rxBuf[0] & 0x1F) == RF_HDR_PAGING_RESPONSE_3X )
    {
        memset(response, 0, sizeof(PAGE_RESPONSE));
        memcpy(&response->DeviceId, rxBuf+1, 4);
        if( response->DeviceId == DeviceId )
        {
            memcpy(&response->StarId, rxBuf+5, 2);
            response->Slot = rxBuf[7];
            response->Offset = rxBuf[8];
            response->Profile1 = rxBuf[9];
            response->Profile2 = rxBuf[10];
            memcpy(&response->Profile3, rxBuf+11, 2);
            memcpy(&response->ProfileEx, rxBuf+13, 4);
            blnRes = TRUE;
        }
    }
    return blnRes;
}

BOOL RF_ReceiveAck(DWORD DeviceId, ACK* ack)
{
    BYTE* rxBuf, pktLen;
    BOOL blnRes = FALSE;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket( rxBuf, MAX_PKT_LEN, 0 );
    if( pktLen == STAR_ACK_PKT_LEN_EX && (rxBuf[0] & 0xF) == RF_HDR_ACK_EX )
    {
        memcpy(&ack->DeviceId, rxBuf+1, 3);
        if( ack->DeviceId == DeviceId )
        {
            memcpy(&ack->Command2X, rxBuf+4, 2);

            ack->Slot = rxBuf[6];

            ack->Offset = rxBuf[7];

            blnRes = TRUE;
        }
    }
    return blnRes;
}

BOOL RF_ReceiveAck3x(DWORD DeviceId, ACK* ack)
{
    BYTE* rxBuf, pktLen;
    BOOL blnRes = FALSE;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket( rxBuf, MAX_PKT_LEN, 5 );
    if( pktLen == STAR_ACK_PKT_LEN_3X && (rxBuf[0] & 0xFF) == RF_HDR_ACK_3X )
    {
        memcpy(&ack->DeviceId, rxBuf+1, 4);
        if( ack->DeviceId == DeviceId )
        {
            ack->Command = rxBuf[5];

            memcpy(&ack->cmdValue, rxBuf+6, 2);

            ack->Slot = rxBuf[8];

            ack->Offset = rxBuf[9];

            blnRes = TRUE;
        }
    }
    else if( pktLen == STAR_MINIMAL_ACK_PKT_LEN_3X && (rxBuf[0] & 0xFF) == RF_HDR_MINIMAL_ACK_3X )
    {
        memcpy(&ack->DeviceId, rxBuf+1, 4);
        if( ack->DeviceId == DeviceId )
        {
            ack->Slot = rxBuf[5];

            ack->Offset = rxBuf[6];

            ack->Command = 0;
            ack->cmdValue = 0;
            blnRes = TRUE;
        }
    }
    return blnRes;
}

////////////////////////////////////////////////////////////////////////////////
//
// Location Info Packet Structure 11 Bytes
//
// ABBB CCCC DDDD DDDD DDDD DDDD DDDD DDDD EEEE EEEE EEEE EEEE
// FFFF GGGG GGGG GGGG HHHH IJKK LLLL MMMM MMMM MMMM

// A: Is Hygiene Room.
// B: Tag Type ( 0: Regular Tag, 1: Hygiene Tag, 2: Temperature Tag, 3: ERU Tag)
// C: Packet Header
// D: TagId
// E: StarId
// F: Command
// G: Command Specific Value
// H: Keys
// I: I am Alive
// J: Motion Flag
// K: Retry
// L: Data Index
// M: RoomId
////////////////////////////////////////////////////////////////////////////////
VOID RF_SendLocationInfo(PKT_LocationData* data)
{
    BYTE* txBuf = buf;

    txBuf[0] = TAG_INFO_LEN_EX;  // Packet length

    txBuf[1] = RF_HDR_TAG_INFO_EX;

    if( data->blnHygieneRoom ) txBuf[1] |= 0x80;

    txBuf[1] |= ((data->Type & 0x7) << 4);

    memcpy( txBuf+2, &data->TagId, 3 );

    memcpy(txBuf+5, &data->LockedStarId, 2);

    memcpy(txBuf+7, &data->CommandResponse, 2);

    txBuf[9] = data->Status;

    data->RoomId |= ((WORD)data->DataIdx << 12);
    memcpy( txBuf+10, &data->RoomId, 2 );

    // Send RF Packet
    RF_SendPacket(txBuf);
}

VOID RF_SendLocationInfo3x(PKT_LocationData* data)
{
    BYTE* txBuf = buf;

    txBuf[0] = TAG_INFO_LEN_EX_3X;  // Packet length

    txBuf[1] = RF_HDR_TAG_INFO_3X_TYPE8;

    txBuf[2] = (data->Type);

    memcpy( txBuf+3, &data->TagId, 4 );

    data->RoomId |= ((WORD)data->blnHygieneRoom << 14);
    memcpy( txBuf+7, &data->RoomId, 2 );

    // Send RF Packet
    RF_SendPacket(txBuf);
}

////////////////////////////////////////////////////////////////////////////////
//
// Monitor I Am Alive Packet Structure 11 Bytes
// XAAA BBBB CCCC CCCC CCCC CCCC CCCC CCCC DDDD DDDD DDDD DDDD
// EEEE FFFF FFFF FFFF XXGG HHHH XIII JJJJ JJJJ JJJJ
//
// A: Monitor Type ( Regular Monitor /  DIM )
// B: Header
// C: Monitor Id
// D: StarId
// E: Command
// F: Command specific Value
// G: Retry
// H: DataIndex
// I: PowerLevel
// J: RoomId
// X: Free Bits
////////////////////////////////////////////////////////////////////////////////
VOID RF_SendMonitorData(BYTE Type, DWORD MonitorId, WORD StarId, WORD cmdResponse,
                        BYTE Status, BYTE Keys, WORD RoomId)
{
    BYTE* txBuf = buf;

    txBuf[0] = MONITOR_INFO_LEN_EX;  // Packet length

    txBuf[1] = RF_HDR_MONITOR_INFO_EX;
    txBuf[1] |= ((Type & 7) << 4);

    memcpy( txBuf+2, &MonitorId, 3 );

    memcpy( txBuf+5, &StarId, 2 );

    memcpy( txBuf+7, &cmdResponse, 2 );

    txBuf[9] = Status;

    RoomId = ((Keys & 0x1) << 15) | (RoomId & 0xFFF);
    memcpy( txBuf+10, &RoomId, 2 );

    RF_SendPacket(txBuf);
}

VOID RF_SendMonitorData3x(BYTE Type, DWORD MonitorId, WORD StarId, DWORD cmdResponse,
                          BYTE Status, BYTE Keys, WORD RoomId, BYTE blnSendProfile,
                          BYTE Profile1, BYTE Profile2, WORD Profile3, BOOL blnSendSummaryInfo)
{
    BYTE* txBuf = buf;

    memcpy( txBuf+2, &MonitorId, 4 );

    txBuf[6] = Type;

    memcpy( txBuf+7, &StarId, 2 );

    memcpy( txBuf+9, &cmdResponse, 3 );

    txBuf[12] = Status;

    RoomId  = ((Keys & 0x7) << 12) | (RoomId & 0xFFF);
    memcpy( txBuf+13, &RoomId, 2 );

    if(blnSendSummaryInfo)
    {
        txBuf[0] = MONITOR_INFO_LEN_3X_TYPE3;  // Packet length

        txBuf[1] = RF_HDR_MONITOR_INFO_3X_TYPE3;

        memcpy(txBuf+15, &sInfo.AliveDataWithRetries, 3);

        memcpy(txBuf+18, &sInfo.AliveDataWithoutRetries, 3);

        memcpy(txBuf+21, &sInfo.PagingData, 3);

        memcpy(txBuf+24, &sInfo.LBIMeasurement, 3);

        memcpy(txBuf+27, &sInfo.TriggerCount, 3);

        memcpy(txBuf+30, &sInfo.BeaconReceiveCount, 3);

        txBuf[39] = flash_settings.HardwareVersion;

        txBuf[40] = flash_settings.HardwareRevision;

    }
    else if(blnSendProfile)
    {
        txBuf[0] = MONITOR_INFO_LEN_3X_TYPE2;  // Packet length

        txBuf[1] = RF_HDR_MONITOR_INFO_3X_TYPE2;

        txBuf[15] = Profile1;
        txBuf[16] = Profile2;
        memcpy(txBuf+17, &Profile3, 2);
    }
    else
    {
        txBuf[0] = MONITOR_INFO_LEN_3X_TYPE1;  // Packet length

        txBuf[1] = RF_HDR_MONITOR_INFO_3X_TYPE1;

    }
    RF_SendPacket(txBuf);
}

BOOL RF_ReceiveFirmwareSegment(FIRMWARE_INFO* FirmwareInfo)
{
    BYTE *rxBuf;
    BYTE pktLen, offset = 1;
    DWORD segInfo;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket(rxBuf, MAX_PKT_LEN, 10);

    if( (pktLen == FW_PKT_LEN && (rxBuf[0] & 0xF) == RF_HDR_FW_PACKET) )
    {
        FirmwareInfo->deviceType = (rxBuf[0] >> 4) & 0xF;

        memcpy(&segInfo, rxBuf+offset, 4);
        offset += 4;

        FirmwareInfo->totSeg = segInfo & 0x3F;
        FirmwareInfo->totSeg = (((FirmwareInfo->totSeg - 1) * 512) / 32);
        FirmwareInfo->segIdx = (segInfo >> 6) & 0x3FF;
        FirmwareInfo->receivedPart = (segInfo >> 16) & 0x1;

        memcpy(FirmwareInfo->segBuf, rxBuf+offset, MAX_UPGRADE_SEGMENT_SIZE);
        offset += MAX_UPGRADE_SEGMENT_SIZE;

        return TRUE;
    }
    return FALSE;
}

BOOL RF_ReceiveHygieneData(WORD DeviceId, DWORD* TagId)
{
    BYTE* rxBuf, pktLen;
    BOOL blnRes = FALSE;
    WORD DeviceIdInACK = 0;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket( rxBuf, MAX_PKT_LEN, 5 );
    if( pktLen == HYGIENE_DATA_PKT_LEN && (rxBuf[0] & 0xF) == RF_HDR_HYGIENE_DATA )
    {
        memcpy(&DeviceIdInACK, rxBuf+1, 2);
        if( DeviceIdInACK == DeviceId )
        {
            memcpy(TagId, rxBuf+3, 3);
            blnRes = TRUE;
        }
    }
    return blnRes;
}

BOOL RF_ReceiveHygieneData3x(WORD DeviceId, DWORD* TagId)
{
    BYTE* rxBuf, pktLen;
    BOOL blnRes = FALSE;
    WORD DeviceIdInACK = 0;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket( rxBuf, MAX_PKT_LEN, 7 );
    if( pktLen == HYGIENE_DATA_PKT_LEN_3X && (rxBuf[0] & 0xFF) == RF_HDR_HYGIENE_DATA_3X )
    {
        memcpy(&DeviceIdInACK, rxBuf+1, 2);
        if( DeviceIdInACK == DeviceId )
        {
            memcpy(TagId, rxBuf+3, 4);
            blnRes = TRUE;
        }
    }
    return blnRes;
}

VOID RF_SendHygieneACK(DWORD TagId)
{
    BYTE* txBuf = buf;

    txBuf[0] = HYGIENE_ACK_PKT_LEN;  // Packet length
    txBuf[1] = RF_HDR_HYGIENE_ACK;
    memcpy( txBuf+2, &TagId, 3 );

    // Send RF Packet
    RF_SendPacket(txBuf);
}

VOID RF_SendHygieneACK3x(DWORD TagId)
{
    BYTE* txBuf = buf;

    txBuf[0] = HYGIENE_ACK_PKT_LEN_3X;  // Packet length
    txBuf[1] = RF_HDR_HYGIENE_DATA_3X;
    memcpy( txBuf+2, &TagId, 4 );

    // Send RF Packet
    RF_SendPacket(txBuf);
}

VOID RF_TurnOn()
{
    CC_DRV_Strobe(CCxxx0_SRX);
}

VOID RF_TurnOff()
{
    // Disable RX and goto IDLE
    CC_DRV_Strobe(CCxxx0_SIDLE);

    // Enter Power Down Mode(Sleep State)
    CC_DRV_Strobe(CCxxx0_SPWD);
}

VOID RF_SendPacket2Upgrade(DWORD DeviceId, WORD segtoupgrade, BYTE UpgradeType, BYTE deviceType, BYTE Part, WORD StarId)
{
    BYTE* txBuf = buf;

    txBuf[0] = MONITOR_INFO_LEN_3X_PKTUPGRADE;  // Packet length

    txBuf[1] = RF_HDR_MONITOR_INFO_3X_PKTUPGRADE;

    memcpy( txBuf+2, &DeviceId, 4 );

    txBuf[6] = deviceType;

    memcpy(txBuf+7, &StarId, 2);

    txBuf[9] = UpgradeType;

    txBuf[10] = Part;

    memcpy(txBuf+11, &segtoupgrade, 2);

    // Send RF Packet
    RF_SendPacket(txBuf);

}

BOOL RF_ReceivePktUpgrade3X(DWORD DeviceId, FIRMWARE_INFO* FirmwareInfo)
{
    BYTE *rxBuf;
    BYTE pktLen, offset = 1;
    DWORD segInfo;
    DWORD recdevId = 0;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket(rxBuf, MAX_PKT_LEN, 12);

    if( (pktLen == FWR_PKTUPGRADE_LEN_3X && (rxBuf[0] & 0xFF) == RF_HDR_FW_PKTUPGRADE) )
    {

        memcpy(&recdevId,  rxBuf+offset, 4);
        offset +=4;

        if( recdevId == DeviceId )
        {
            FirmwareInfo->deviceType  = rxBuf[offset++]  & 0xFF;

            memcpy(&segInfo, rxBuf+offset, 4);
            offset += 4;

            FirmwareInfo->totSeg = segInfo & 0x3F;
            FirmwareInfo->totSeg = (((FirmwareInfo->totSeg - 1) * 512) / 32);  // total segment plus 1 for 2 interrupt vector
            FirmwareInfo->segIdx = (segInfo >> 6) & 0x3FF;
            FirmwareInfo->receivedPart = (segInfo >> 16) & 0x1;
            FirmwareInfo->UpgradeType = (segInfo >> 17) & 0x1;

            memcpy(FirmwareInfo->segBuf, rxBuf+offset, MAX_UPGRADE_SEGMENT_SIZE);
            offset += MAX_UPGRADE_SEGMENT_SIZE;

            return TRUE;
        }
    }
    return FALSE;
}

BOOL RF_ReceiveFirmwareSegment3X(FIRMWARE_INFO* FirmwareInfo)
{
    BYTE *rxBuf;
    BYTE pktLen, offset = 1;
    BYTE segInfo;

    rxBuf = buf;
    memset(rxBuf, 0, MAX_PKT_LEN);

    pktLen = RF_ReceivePacket(rxBuf, MAX_PKT_LEN, 9);

    if( (pktLen == FW_PKT_LEN_EX && (rxBuf[0] & 0xFF) == RF_HDR_FW_PACKET) )
    {
        FirmwareInfo->deviceType  = rxBuf[offset++]  & 0xFF;

        FirmwareInfo->totSeg = rxBuf[offset++] & 0x3F;
        FirmwareInfo->totSeg = (((FirmwareInfo->totSeg - 1) * 512) / 32);  //// total segment plus 1 for 2 interrupt vector

        memcpy(&FirmwareInfo->segIdx, rxBuf+offset, 2);
        offset+=2;

        segInfo = rxBuf[offset++];
        FirmwareInfo->receivedPart = segInfo & 0x1;
        FirmwareInfo->UpgradeType = (segInfo >> 1) & 0x1;

        FirmwareInfo->HardwareVersion = rxBuf[offset++];

        memcpy(FirmwareInfo->segBuf, rxBuf+offset, MAX_UPGRADE_SEGMENT_SIZE);
        offset += MAX_UPGRADE_SEGMENT_SIZE;

        return TRUE;
    }
    return FALSE;
}
