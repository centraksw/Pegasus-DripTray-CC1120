#include "defines.h"
#include "rf.h"
#include "rf_app.h"
#include "profile.h"
#include "io.h"
#include "timer_drv.h"
#include <string.h>
#include "flash_app.h"
#include "dfu.h"
#include "general.h"
#include "lf.h"
#include "motion.h"
#include "dim.h"
#define HHC_INTERVAL    12300

/* Paging Variables */
PAGE_RESPONSE response;
SUMMARY_INFO sInfo;

BOOL blnPseudoSync;
BYTE PacketRequestCnt = 0;

BYTE LocationDataIndex=0;
BYTE IAmAliveRetry = 1;
BYTE MonitorDataFailCnt = 0;
BYTE PseudoSycnRetryCnt=0;
WORD LBIValue = 0;
BYTE RepeatRetry = 0;
BYTE IRCount = 0;
BOOL blnTriggerStatus = FALSE;
BYTE triggerDelay = 0;
DWORD cmdReply = 0;
static DWORD TagId;
static BOOL blnSetFrequency = FALSE;
static BYTE delayIdx=0;
static ACK ack;
static BYTE PageIndex;
static BYTE Retry = 0;
BOOL blnTriggered = FALSE;
BOOL blnpkttoupgrade = FALSE;

BOOL blnCheckupgradePktreq = FALSE;
BYTE SpecialRFdelay[3];

static BYTE PendingKeys = 0;

static WORD GetMedian(WORD* ADCVal, BYTE cnt)
{
    BYTE i, j;
    WORD T;
    for(i=0; i<cnt-1; i++)
    {
        for(j=i+1; j<cnt; j++)
        {
            if( ADCVal[i] > ADCVal[j] )
            {
                T = ADCVal[i];
                ADCVal[i] = ADCVal[j];
                ADCVal[j] = T;
            }
        }
    }
    return ADCVal[cnt/2];
}

// Delay used before Location data send and Paging data send
// Delay is used as raw value
// Delay is in the rage of 30 ms to 220 ms
// MAX delays is 16
static BYTE GetDelayVal()
{
    BYTE delayVal;

    delayVal = flash_settings.delay[delayIdx];
    // Increament the delay index to pick the next random delay.
    ++delayIdx;

    if( delayIdx >= MAX_DELAY_VALUES )
        delayIdx = 0;

     if(delayVal > 220)
      delayVal -=10;

    return delayVal;
}


// Check if the freq is within range 1 to 6
// Write Freq index and sleep sate to flash
static VOID OnSetFrequency(BYTE fIdx)
{
    if( fIdx >= 1 && fIdx <= MAX_FREQUENCY && fIdx != flash_settings.FreqIdx )
    {
        flash_settings.FreqIdx = fIdx;
        FLASH_APP_Write();
        blnSetFrequency = TRUE;
    }
}

// Process MONITOR Command Received from Star.
// Command format:  4 bit command; 12 bit data
static VOID ProcessCommand(WORD Command)
{
    BYTE cmd = ( Command >> 12 ) & 0xF;
    WORD data =  Command & 0xFFF;

    // When the next data is sent to the star, the previous command will be sent back
    // Save the command that we received this time so we can do this
    cmdReply = Command;

    Command &= 0xF000;

    // Take action on the command
    switch( cmd )
    {
    case CMD_MONITOR_RESET:
        Reset(0);
        break;

    case CMD_MONITOR_SET_FREQUENCY:
        OnSetFrequency(data);
        break;

    case CMD_MONITOR_GET_PROFILE:
        cmdReply = Command | PROFILE_GetProfile();
        break;

    case CMD_MONITOR_GET_VERSION:
        cmdReply = Command | PROFILE_GetVersion();
        break;

    case CMD_MONITOR_GET_BATTERY_STATUS:
        cmdReply = Command | LBIValue;
        break;
    }
}
// Process MONITOR Command Received from Star.
// Command format:  4 bit command; 12 bit data
static VOID ProcessCommand3x(BYTE Command, WORD data)
{
    // When the next data is sent to the star, the previous command will be sent back
    // Save the command that we received this time so we can do this
    cmdReply = ((DWORD)(Command & 0xFF) << 16) | data ;

    // Take action on the command
    switch( Command )
    {
    case CMD_MONITOR_RESET:
        Reset(0);
        break;

    case CMD_MONITOR_SET_FREQUENCY:
        OnSetFrequency(data);
        break;

    case CMD_MONITOR_GET_PROFILE:
        settings.blnSendProfileToPC = TRUE;
        cmdReply =((DWORD)(Command & 0xFF) << 16) | PROFILE_GetProfile();
        break;

    case CMD_MONITOR_GET_VERSION:
        cmdReply = ((DWORD)(Command & 0xFF) << 16) | PROFILE_GetVersion();
        break;

    case CMD_MONITOR_GET_BATTERY_STATUS:
        cmdReply = ((DWORD)(Command & 0xFF) << 16) | LBIValue;
        break;

    case CMD_MONITOR_CLEAR_SUMMARY:
        memset(&sInfo, 0, sizeof(sInfo));
        FLASH_APP_Summary_Write();
        blnSendSummaryInfo = TRUE;
        break;

    case CMD_MONITOR_GET_SUMMARY_INFO:
        cmdReply = (DWORD)(Command & 0xFF) << 16;
        blnSendSummaryInfo = TRUE;
        break;
    }
}

VOID RF_APP_ProcessBCastCommand(BYTE Command)
{
    BYTE cmd = ( Command >> 4 ) & 0xF;
    switch( cmd )
    {
    case BCAST_CMD_WAKEUP:
        blnMonitorWakeup = TRUE;
        break;
    }
}

// Set the current antenna state
VOID RF_APP_SwitchAntenna()
{
    if(settings.antenna ==RF_ANTENNA1)
        settings.antenna = RF_ANTENNA2;
    else
        settings.antenna = RF_ANTENNA1;
}

// Once we receive the amount of time to wait for the beacon from the star, we will sleep for that much time
BYTE RF_APP_WaitForBeacon(BYTE slot, BYTE offset)
{
    // Time remaining in the last slot - just wait for this time
    INT32 waitTime = offset;

    // Stop WDT
    TIMER_StopWDT();

    // Disable unused IO Pins.
    IO_ShortSleep_Enter();

    // Sleep  in LPM1 ( remaining time of the current slot )
    if( waitTime > 0 )
        TIMER_DelayMS( waitTime );

    // Enables the required IO Pins.
    IO_ShortSleep_Exit();

    //Start the WDT. Now WDT will be synced with Star WDT.
    TIMER_InitWDT();

    slot = slot+1;
    if( slot > MAX_SLOTS )
    {
        slot = 1;
        blnBeaconReceived = FALSE;
    }

    // Send the current slot number
    return slot;
}

static BOOL SendIAmAlive(BOOL blnNoDelay)
{
    BYTE status;
    BOOL blnRes=FALSE;
    BYTE delayVal=0;

    PendingKeys |= KeyPressed;
    KeyPressed = 0;

    // Delay a random amount here Put the micro in LPM1
    // Range of delay - 30 ms  to 220 ms
    delayVal = GetDelayVal();

    // If No Sleep Flag is set then reduce the delay 10 times.
    if( blnNoDelay )
        delayVal /= 10;

    TIMER_DelayMS( delayVal );

    WORD UpgradedPktsCnt;
    if( settings.blnFirmwareUpgrade )
        UpgradedPktsCnt = DFU_GetUpgradedPacketsCount();

    // Select Channel
    RF_SetChannel( settings.Data_Channel);

    // Select Antenna
    RF_SetAntenna( settings.antenna );

    // Set Tx power
    RF_SetTXPower(PLUS_3_DBM);

    // If there is command that was issued in the last cycle and there is a reply waiting
    // send that reply now. If not automatically send the Profile and the Raw LBI value every other time
    if( cmdReply == 0 )
    {
        if(!settings.bln30System)
        {
            // Every odd time send this
            switch(LocationDataIndex % 3)
            {
            case 0: cmdReply = ((WORD)CMD_MONITOR_GET_BATTERY_STATUS << 12) | LBIValue; break;
            case 1: cmdReply = ((WORD)CMD_MONITOR_GET_PROFILE << 12) | PROFILE_GetProfile(); break;
            case 2: cmdReply = ((WORD)CMD_MONITOR_GET_VERSION << 12) | PROFILE_GetVersion(); break;
            }

            if( settings.blnFirmwareUpgrade )
                cmdReply = ((WORD)CMD_MONITOR_UPGRADE_INFO << 12) | UpgradedPktsCnt;
        }
        else
        {
            if( settings.blnFirmwareUpgrade )
                cmdReply = ((DWORD)CMD_MONITOR_UPGRADE_INFO << 16) | UpgradedPktsCnt;
            else
                switch(LocationDataIndex % 3)
                {
                case 0: cmdReply = ((DWORD)CMD_MONITOR_GET_BATTERY_STATUS << 16) | LBIValue; break;
                case 1: cmdReply = ((DWORD)CMD_MONITOR_GET_PROFILE << 16) | PROFILE_GetProfile(); break;
                case 2: cmdReply = ((DWORD)CMD_MONITOR_GET_VERSION << 16) | PROFILE_GetVersion(); break;
                }
        }
    }
    status = (LocationDataIndex & 0xF) | ((IAmAliveRetry & 0x3) << 4);
    if( blnTriggered ) status |= 0x80;

    // Disables motion Interrupt
    MD_Disable();

    // Send data
    if(settings.bln30System)
    {
        RF_SendMonitorData3x(MONITOR_TYPE, flash_settings.DIMId_3X, response.StarId, cmdReply, status,  PendingKeys,
                            settings.RoomId, settings.blnSendProfileToPC, settings.Profile1,
                            settings.Profile2, settings.ProfileEx, blnSendSummaryInfo);
        TIMER_InitSlotOffset();
        blnRes = RF_ReceiveAck3x(flash_settings.DIMId_3X, &ack);
    }
    else
    {
        RF_SendMonitorData(MONITOR_DIM, flash_settings.DIMId_2X, response.StarId, cmdReply, status, PendingKeys, settings.RoomId);

         TIMER_InitSlotOffset();
        // Receive Ack
        blnRes = RF_ReceiveAck(flash_settings.DIMId_2X, &ack);
    }

    sInfo.AliveDataWithRetries += 1;

     // Enables motion Interrupt
     MD_Enable();
     DWORD recoffset = TIMER_GetSlotOffsetInUS();

    // If we received an ACK from the star
    if( blnRes )
    {
        settings.blnSendProfileToPC = FALSE;
        sInfo.AliveDataWithoutRetries += 1;
        blnSendSummaryInfo = FALSE;
        cmdReply = 0;

        PendingKeys = 0;

        // If set frequency command received from STAR, send Command ack and then reset the tag.
        if(blnSetFrequency)
        {
            blnSetFrequency = FALSE;
            Reset(0);
        }

        if( beaconFailCnt > 0 )
        {
            recoffset = (recoffset+499)/1000;
            if( ack.Offset > recoffset )
                ack.Offset = ack.Offset - recoffset;

            curSlot = RF_APP_WaitForBeacon(ack.Slot, ack.Offset);
        }

        // Command will be sent to us in the ACK packet
        // Process the command sent by the PC
        if(settings.bln30System)
            ProcessCommand3x(ack.Command, ack.cmdValue);
        else
            ProcessCommand(ack.Command2X);
    }
    else
    {
        // If we do not get ACK from the star, we switch the antenna
        RF_APP_SwitchAntenna();
    }
    return blnRes;
}

// Send Paging Request and check response
BOOL RF_APP_DoPaging(BOOL bln30System)
{
    BOOL blnRes;
    WORD Value;
    BYTE part;
    DWORD recv_offset;

    if(bln30System)
        RF_Init(BRATE_50_KBPS);
    else
        RF_Init(BRATE_76_KBPS);

    // Random delay - 30 ms to 220 ms.
    //Put the micro in LPM1 mode.
    TIMER_DelayMS( GetDelayVal() );

    // Set paging channel
    RF_SetChannel(settings.Data_Channel);

    // Set Tx Power
    RF_SetTXPower(PLUS_3_DBM);

    // Set RF antenna
    RF_SetAntenna( settings.antenna );

    part = DFU_GetCurrentWorkingPart();

    Value = (part == PART1) ? (PART1-1) : (PART2-1);

    // Page index will be reset after the full paging sequence is complete
    ++PageIndex;

    // Disable motion sensor.
    MD_Disable();

    if(!bln30System)
    {
        Value <<= 3;
        Value |= MONITOR_DIM;
        Value <<= 10;
        Value |= (LBIValue & 0x3FF);

        // Send Paging Request
        RF_SendPagingRequest(flash_settings.DIMId_2X, Value, VERSION, PageIndex);

        TIMER_InitSlotOffset();
        // Receive Paging Response
        blnRes = RF_ReceivePagingResponse(flash_settings.DIMId_2X, &response);
    }
    else
    {
        // Send Paging Request
        RF_SendPagingRequest3x(flash_settings.DIMId_3X, Value, VERSION, PageIndex, 0, blnPseudoSync);

        TIMER_InitSlotOffset();
        // Receive Paging Response
        blnRes = RF_ReceivePagingResponse3x(flash_settings.DIMId_3X, &response);
    }
    sInfo.PagingData += 1;

    if( blnRes )
    {
        recv_offset = TIMER_GetSlotOffsetInUS();
        recv_offset = (recv_offset + 499) / 1000;
        if( response.Offset > recv_offset )
            response.Offset = response.Offset - recv_offset;
    }
    else
    {
        // Switch antenna if there is no Paging response.
        RF_APP_SwitchAntenna();
    }

    // Enable Motion sensor.
    MD_Enable();

    // Disable Unused IO Pins.
    IO_ShortSleep_Enter();

    // Return Paging Status. True if response received otherwise false.
    return blnRes;
}


// Send out a Page request and receives a Page reply
VOID RF_APP_PagingProcess()
{
    BYTE state = RF_STATE;
    BYTE pageInterval, PagingCnt = 0;
    BOOL bln30Paging = FALSE;
    blnSendAliveData = FALSE;
    Retry = 1;
    // Initialize Page index
    PageIndex = 0;

    bln30Paging  = settings.bln30System;

    TIMER_InitWDT();
    pageInterval = PAGE_DELAY_250_MS;
    g_PageCounter = pageInterval;

    while(TRUE)
    {
        if( g_PageCounter > pageInterval )
        {
            g_PageCounter = 0;

            // Good response - then stop
            if( RF_APP_DoPaging(bln30Paging) )
            {
                break;
            }
            // First Four times send the paging request in 250ms + random interval gap
            // After that one times send the paging request in 3 sec + random interval gap
            // After that send the paging request in 12.5 sec + random interval gap
            // Random delay will be in the DoPaging() routine - here we will sleep the rest (in multiples of 250 ms)
            if(!flash_settings.blnDIMInSystem)
            {
                if (Retry <= 3)
                {
                    Retry += 1;
                    pageInterval = PAGE_DELAY_1_5_SEC;
                }
                else if(Retry == 4 || Retry == 6)
                {
                    pageInterval = PAGE_DELAY_12_5_SEC;
                    Retry += 1;
                }
                else if(Retry == 5 || Retry == 7)
                {
                    pageInterval = PAGE_DELAY_6_SECS;

                    if (Retry == 7)
                        Retry = 6;
                    else
                        Retry += 1;
                }
            }
            else if(flash_settings.blnDIMInSystem)
            {
                if(PagingCnt > 9 && PagingCnt < 11 )
                {
                    bln30Paging = settings.bln30System;
                }
                else if(PagingCnt > 10)
                {
                    PagingCnt = 0;
                    bln30Paging = settings.bln30System;
                }
                if (Retry <= 3 )
                {
                    Retry += 1;
                    pageInterval = PAGE_DELAY_1_5_SEC;
                    PagingCnt = 2;
                }
                else if(Retry == 4)
                {
                    pageInterval = PAGE_DELAY_12_5_SEC;
                    Retry = 6;
                    if(PagingCnt > 9)
                        Retry = 5;

                }
                else if(Retry == 5 )
                {
                    Retry = 6;
                    if(settings.bln30System)
                    {
                        if( g_BatteryCounter > LOW_BATTERY_CYCLE_COUNTER_3X )
                        {
                            sInfo.LBIMeasurement += 1;
                            LBIValue = RF_APP_GetBatteryStatus();
                            RF_APP_AverageLBIMeasurement(LBIValue);
                        }
                    }
                    else
                    {
                        if( g_BatteryCounter > LOW_BATTERY_CYCLE_COUNTER )
                        {
                            LBIValue = RF_APP_GetBatteryStatus();
                            RF_APP_AverageLBIMeasurement(LBIValue);
                        }
                    }
                    if( g_SummaryWriteCounter > SUMMARY_WRITE_COUNTER )
                    {
                        g_SummaryWriteCounter = 0;
                        FLASH_APP_Summary_Write();
                    }
                    pageInterval = PAGE_DELAY_6_SECS;
                }
                else if( Retry == 6 )
                {
                    Retry = 4;
                    ++PagingCnt;
                     pageInterval = PAGE_DELAY_6_SECS;
                }
            }
        }

        if( DIM_CheckIsTriggered(settings.Profile) && IRCount==0 && triggerDelay==0 )
        {
            IRCount = 6;
            blnTriggered = TRUE;
            blnTriggerStatus = TRUE;
            sInfo.TriggerCount += 1;

            blnSendAliveData = TRUE;
            IAmAliveRetry = 1;
            g_LocationCounter = MONITOR_I_AM_ALIVE_INTERVAL;
        }

        state = GetState(curSlot) ;
        switch( state )
        {
        case PC_COM_RES_STATE:
        case RF_STATE:
             DimIRRFHandler(RF_STATE, TRUE);
            break;
        }

        if( triggerDelay > 0 )
            triggerDelay -= 1;

        if( (IRCount == 0) && (triggerDelay==0) )
            DIM_IsProximityTriggered(settings.Profile);

         if( IO_ScanKeys() )
             KeyPressed =  1;

        IO_ShortSleep_Enter();

        Micro_Sleep();
        // Enable the required IO Pins.
        IO_ShortSleep_Exit();
    }
}

BOOL RF_APP_SendMonitorData(BOOL blnNoDelay)
{
    // If Send Location data flag is not set, then no need to send the location data.
     if( !blnSendAliveData || ReadyToSendLocationDataCnt > 0 ) return FALSE;

    if( SendIAmAlive(blnNoDelay) )
    {
        MonitorDataFailCnt = 0;

        LocationDataIndex += 1;
        blnSendAliveData = FALSE;
        if( cmdReply == 0 )
        {
            g_LocationCounter = 0;
            g_UpgradeLocationCounter = 0;
            blnMonitorWakeup = FALSE;

            blnTriggered = FALSE;
        }
        return TRUE;
    }

    // If we tried three times, then increment the Location data fail count. and location data index.
    if( ++IAmAliveRetry > MAX_MONITOR_INFO_RETRY )
    {
        MonitorDataFailCnt += 1;
        LocationDataIndex += 1;
        blnSendAliveData = FALSE;
        g_LocationCounter = 0;
        g_UpgradeLocationCounter = 0;
        blnTriggered = FALSE;
        blnMonitorWakeup = FALSE;
    }
    return FALSE;
}

#define  UPGRADE_RX_TIME_OUT_LESSTHAN_STAR_144                    130000l
#define  UPGRADE_RX_TIME_OUT_GREATERTHAN_STAR_144                 5000l
DWORD RF_APP_ReceiveUpgradeFirmware(DWORD Timeoffset)
{
    DWORD Offset, time;
    DWORD UpgradeRxoffset = Timeoffset;
    BOOL blnRes = FALSE, blnExit = FALSE;

    BYTE cnt = 0;
    BYTE part = 0;

    TIMER_InitSlotOffset();
    if(settings.bln30System)
        RF_Switch_Bitrate(BRATE_150_KBPS);

    RF_SetAntenna(settings.antenna);

    RF_SetChannel(settings.Beacon_Channel);



    Offset = TIMER_GetSlotOffsetInUS();
    UpgradeRxoffset += Offset;
    if(settings.bln30System)
    {
    	if(DFU_GetUpgradeSlot(response.StarId) <= 144 || (isBeaconSlotAfterIRState(LockedBeaconSlot)))
        	time = UPGRADE_RX_TIME_OUT_LESSTHAN_STAR_144;
    	else
        	time = UPGRADE_RX_TIME_OUT_GREATERTHAN_STAR_144;

    	if ( UpgradeRxoffset  < time )
    	{
        	TIMER_DelayUS( time-UpgradeRxoffset);
       	 	Offset += (time - UpgradeRxoffset) ;
    	}
    }
    TBCTL = TBSSEL_1 + MC_2 + TBCLR;
    TBCCTL1 = 0;
    TBCCTL1 = CCIE;

    while(TRUE)
    {
        // Set a CCR to calculate Timeout and turn of the CCR Interrupt
        if(settings.bln30System)
            TBCCR1 = TBR + 426;   // 13 ms time out (13000/30.51 = 426)
        else
            TBCCR1 = TBR + 656;   // 20 ms time out (20000/30.51 = 656)

        memset(FirmwareInfo.segBuf, 0, sizeof(FirmwareInfo.segBuf));

        // Disable motion sensor
        MD_Disable();

        if( settings.bln30System )
            blnRes = RF_ReceiveFirmwareSegment3X(&FirmwareInfo);
        else
            blnRes = RF_ReceiveFirmwareSegment(&FirmwareInfo);


        // Disable motion sensor
        MD_Enable();

        part = DFU_GetCurrentWorkingPart();
        if( blnRes && (((FirmwareInfo.deviceType & 0x7) == MONITOR_DIM) || (FirmwareInfo.deviceType == MONITOR_TYPE)) && ((FirmwareInfo.receivedPart+1) != part))
        {
            if( settings.bln30System && flash_settings.HardwareVersion != FirmwareInfo.HardwareVersion )
                DFU_Stop();
            FailedToReceiveUpgradePktCnt = 0;

            DFU_WriteFirmware();
        }
        else
            FailedToReceiveUpgradePktCnt += 1;

        if(settings.bln30System)
        {
            Offset += UPGRADE_TIMEGAP_3X;
            if( ++segmentCompleted  >= FirmwareInfo.totSeg )
                blnCheckupgradePktreq = TRUE;
        }
        else
            Offset += UPGRADE_TIMEGAP;

        // calculate the time consumed to receive and store the firmware packet in flash memory.
        // Wait to receive the next firmware packet.
        // Each firmware packet will be sent in 20 ms interval.
        MICRO_Sleep_LPM1();

        cnt++;

        // Stop firmware upgrade process, if 10 firmware packets failed consecutively
        if(settings.bln30System)
        {
	    // Stop firmware upgrade process, if 10 firmware packets failed consecutively
            if( FailedToReceiveUpgradePktCnt >= MAX_NO_UPGRADE_PKT_RECEIVED_3X )
            {
                DFU_Stop();
                g_UpgradeLocationCounter = DIM_UPGRADE_PACKET_TIMEOUT;
            }
            // Exit the firmware receive if all the 8 packets received.
            if( cnt >= UPGRADE_PKT_PER_CYCLE_3X ) break;
        }
        else
        {
            if( FailedToReceiveUpgradePktCnt >= MAX_NO_UPGRADE_PKT_RECEIVED )
                DFU_Stop();

            if( cnt >= UPGRADE_PKT_PER_CYCLE_2X ) break;
        }
        // Exit the firmware receive if the erase segment flag is set.
        // If the erase segment flag is set, we have to erase the entire 512 byte segment.
        // So no need to receive the other packet in the current cycle.
        if( blnEraseSegment )
            break;

        if( blnExit )
            break;
    }

    TBCCTL1 = 0;


    if( settings.bln30System )
    {
        RF_Init(BRATE_50_KBPS);
        Offset += 3100l; // adjust made for offset for 400 us to sync (Previous value is 3500l)
    }
    return Offset;
}

static BOOL ReceiveHygieneACK()
{
    BOOL blnRes = FALSE;
    BYTE idx;
    DWORD offset;

    RF_SetChannel(settings.Beacon_Channel);

    RF_SetAntenna(settings.antenna);

    RF_SetTXPower(MINUS_20_DBM);

    TagId = 0;
    for( idx=0; idx<4; idx++)
    {
        TIMER_InitSlotOffset();

        // Diable motion sensor before going to RF
        MD_Disable();

        if(settings.bln30System)
        {
            if( RF_ReceiveHygieneData3x(settings.RoomId, &TagId) )
            {
                RF_SendHygieneACK3x(TagId);
                blnRes = TRUE;
            }
        }
        else
        {
            if( RF_ReceiveHygieneData(settings.RoomId, &TagId) )
            {
                RF_SendHygieneACK(TagId);
                blnRes = TRUE;
            }
        }
        // By defaault RF_TurnOff called inside the RF Receive.  Here we are doing RF_Send after RF_Receive.
        RF_TurnOff();
        // Enable Montion sensor
        MD_Enable();

        offset = TIMER_GetSlotOffsetInUS();

        if( offset < HHC_INTERVAL )
        {
            offset = HHC_INTERVAL - offset;
            TIMER_DelayUS(offset);
        }
    }
    return blnRes;
}

VOID DimIRRFHandler(BYTE state, BOOL blnPaging)
{
    if( IRCount > 0 )
    {
        IRCount -= 1;
        TIMER_Init();

        TIMER_DelayMS(15);

        // Enable step up converter.
        LF_STEPUP_CONV_DIR |= LF_STEPUP_CONV;
        LF_STEPUP_CONV_OUT |= LF_STEPUP_CONV;

        TIMER_DelayMS(60);

        MD_Disable();
        LF_Transmit(settings.RoomId,settings.LFRange);
        TACTL = TACLR + TASSEL_1 + MC_2;
        MD_Enable();

        LF_STEPUP_CONV_DIR &= ~LF_STEPUP_CONV;

        if(settings.blnLEDBlink)
            IO_LEDLit(LED_RED, 40);    // Changed the LED_Red as per the OFD Requirement by deepak (task id:10852  IT-374H DIM LEDs)
    	else
            TIMER_DelayMS(40);

        if( ReceiveHygieneACK() )
        {
            MD_Disable();
            if(settings.blnLEDBlink)
              IO_LEDLit(LED_GREEN, 50);
            MD_Enable();
            IRCount = 0;
            RepeatRetry = 0;
        }
        else
        {
            if(!blnPaging )
            {
                //Tag didn't location data when Star in ethernet state
                if( isStarEthernetState(response.StarId) )
                    return;
            }

            TagId = 0;
            if( state==RF_STATE && IAmAliveRetry >= 1)
                RF_APP_SendMonitorData(TRUE);
        }
    }
    else
    {
        if(!blnPaging )
        {
            //Tag didn't location data when Star in ethernet state
            if( isStarEthernetState(response.StarId) )
                return;
        }
        if( state==RF_STATE )
        {
            // Do Pseudo Sync, If Beacon Fail Count reaches 2 or Location fail count reaches 3
            if( (beaconFailCnt >= MAX_MONITOR_PSEUDO_SYNC ||
                 MonitorDataFailCnt >= MAX_MONITOR_DATA_FAIL) && !blnPageReqSent && blnPaging == FALSE && (curSlot % 8) == (flash_settings.DIMId_2X % 8) )
            {
                // Set Page request sent flag to do pseudo sync once in every dutycycle.
                blnPageReqSent = TRUE;

                flash_settings.blnDIMInSystem = TRUE;

                // Increment Pseudo sync retry count.
                PseudoSycnRetryCnt += 1;

                // Do Pesudo Sync Paging once.
                // If ack Received then exit from main loop.
                blnPseudoSync = TRUE;
                if( RF_APP_DoPaging(settings.bln30System) )
                {
                    // Reset Pseudo sync retry count.
                    PseudoSycnRetryCnt = 0;

                    // Set Paging completed flag to ignore full paing.
                    blnPagingCompleted = TRUE;

                    // Set Beacon Fail count to Max value to exit from main loop and to re-sync time.
                    // WDT is reset and new profile is applied in the begining of the main loop.
                    beaconFailCnt = MAX_MONITOR_BEACON_FAIL;
                }
                blnPseudoSync = FALSE;
            }
            else
            {
                RF_APP_SendMonitorData(FALSE);
                RF_APP_GetPackettoUpgrade();
                RF_APP_SendTagData();
            }

            //Reset Pseudo Sync Retry until Beacon Fail Count or location Data Fail count reaches its Max value.
            if( beaconFailCnt < MAX_MONITOR_PSEUDO_SYNC && MonitorDataFailCnt < MAX_MONITOR_DATA_FAIL )
                PseudoSycnRetryCnt = 0;
        }
    }

    if( blnTriggerStatus && IRCount == 0 )
    {
        triggerDelay = 4;
        blnTriggerStatus = FALSE;
    }
}


VOID RF_APP_OnIRRFHandler(BYTE state)
{
    DimIRRFHandler(state, FALSE);
}

// Measure the Battery voltage every 10 minutes
WORD RF_APP_GetBatteryStatus()
{
    WORD ADCVal[9];
    WORD counter;
    WORD LBIADC;
    BYTE idx;

    // Used to keep track of 10 minutes
    g_BatteryCounter = 0;

    // Init the RF chip every 10 minutes
    if(settings.bln30System) //2x
        RF_Init(BRATE_50_KBPS);
    else
        RF_Init(BRATE_76_KBPS);

    // Disable the motion sensor
    MD_Disable();

    // Set voltage out
    BATTERY_ADC_POWER_DIR |= BATTERY_ADC_POWER;
    BATTERY_ADC_POWER_OUT |= BATTERY_ADC_POWER;

    // Configure Input pin
    BATTERY_ADC_INPUT_DIR &= ~BATTERY_ADC_INPUT;

    // Turn on RF power so the battery will be stressed
    RF_TurnOn();

    // Select channel 2 as input for ADC
    ADC10CTL1 = INCH_2;

    ADC10CTL0 = SREF_1 + ADC10SHT_3 + ADC10ON + REFON + REF2_5V;

    // Sleep 5 ms in LPM1
    TIMER_DelayMS(5);

    // Measure Battery voltage 3 times
    for(idx=0; idx<9; idx++)
    {
        // ADC on , ENABLE CONVERSION
        ADC10CTL0 |= ENC + ADC10SC;

        // Wait to see if conversion is complete
        // Repeat this 5000 times if not, just set the LBI Value to 0 and exit
        counter = RESET_TIMEOUT;
        while ( --counter && (ADC10CTL1 & ADC10BUSY) );

        if(counter == 0)
            ADCVal[idx] = 0;
        else
            ADCVal[idx] = ADC10MEM;
    }

    // Wait if ADC10 core is active
    ADC10CTL0 &= ~ENC;

    LBIADC = GetMedian(ADCVal, 9);

    counter = RESET_TIMEOUT;
    while ( --counter && (ADC10CTL1 & BUSY) );

    // Turn off ADC and voltage
    RF_TurnOff();

    // Turn off the ADC
    ADC10CTL1 = 0;
    ADC10CTL0 = 0;

    // Disable ADC IO
    BATTERY_ADC_POWER_OUT &= ~BATTERY_ADC_POWER;

    BATTERY_ADC_INPUT_DIR |= BATTERY_ADC_INPUT;
    BATTERY_ADC_INPUT_OUT &= ~BATTERY_ADC_INPUT;

    // Enable Motions sensor
    MD_Enable();

    return LBIADC & 0x3FF;
}

VOID RF_APP_SendTagData()
{
    if( RepeatRetry >= 3 )
        TagId = 0;

    if( TagId > 0 )
    {
        RF_SetChannel(settings.Data_Channel);

        RF_SetAntenna(settings.antenna);

        RF_SetTXPower(PLUS_3_DBM);

        PKT_LocationData data;

        memset(&data, 0, sizeof(PKT_LocationData));

        data.TagId = TagId;
        data.LockedStarId = 0;
        data.CommandResponse = 0;
        data.Status = 0;
        data.RoomId = settings.RoomId;
        data.DataIdx = 0;
        data.blnHygieneRoom = TRUE;

        // Diable motion sensor before going to RF
        MD_Disable();

        if( settings.bln30System )
        {
            data.Type = TAG_HYGIENE_3x;
            RF_SendLocationInfo3x(&data);
        }
        else
        {
            data.Type = TAG_HYGIENE;
            RF_SendLocationInfo(&data);
        }

        RF_TurnOff();
        // Enable Montion sensor
        MD_Enable();

        RepeatRetry += 1;
    }
}

VOID RF_APP_GetPackettoUpgrade()
{
    if( !blnpkttoupgrade || !settings.blnFirmwareUpgrade )
        return;

     WORD segIdxtoupgrade = 0;

    BYTE part = DFU_GetCurrentWorkingPart();
    BOOL blnRes = FALSE;

    if(!DFU_is5percettoupgrade(FirmwareInfo.segIdx,   FirmwareInfo.totSeg ))
        return;

    if(!DFU_GetSegmenttoupgrade(FirmwareInfo.totSeg, &segIdxtoupgrade))
        return;


    if(++PacketRequestCnt >= 4)
    {
        PacketRequestCnt = 0;
        blnpkttoupgrade = FALSE;
    }

    //If IO_KeyPressed is False, then we should have to use the random delay
    //Otherwise no need to use the random delay.
    TIMER_DelayMS( SpecialRFdelay[PacketRequestCnt]);

    // Select Channel
    RF_SetChannel( settings.Data_Channel );

    RF_SetAntenna( settings.antenna);

    // Set Tx Power
    RF_SetTXPower(PLUS_3_DBM );

    RF_SendPacket2Upgrade(flash_settings.DIMId_3X, segIdxtoupgrade, MONITOR_UPGRADE, MONITOR_TYPE, part, response.StarId);

    blnRes = RF_ReceivePktUpgrade3X(flash_settings.DIMId_3X, &FirmwareInfo);

    if( blnRes && FirmwareInfo.deviceType == MONITOR_TYPE && ((FirmwareInfo.receivedPart+1) != part) )
    {
        FailedToReceiveUpgradePktCnt = 0;

        // Store the received firmware in code memory
        DFU_WriteFirmware();
    }
}
//find the slot index in random delay
static BYTE GetfreeSlotindex(BYTE* buf, BYTE len, BYTE Slot)
{
    int cnt = 0;
    for(int idx=0; idx<len; idx++)
    {
        if( buf[idx] == 0 )
        {
            if( cnt == Slot )
                return idx;
            cnt++;
        }
    }

    return 0;
}
//sort the delay value
static void sort(BYTE* delay )
{
    BYTE temp =0;
    for(BYTE i=0; i<3; i++)
    {
        for(BYTE j=i; j<3; j++)
        {
            if( delay[i] > delay[j] )
            {
                temp = delay[i];
                delay[i] = delay[j];
                delay[j] = temp;
            }
        }
    }
}

//Find and set the delay value
void SetDelayValue(BYTE* delay)
{
    BYTE delay1, delay2, delay3;
    delay1 = delay[0];
    delay2 = (delay[1]-delay[0])-1;
    delay3 = (delay[2]-delay[1])-1;

    delay[0] = delay1*10;
    delay[1] = delay2*10;
    delay[2] = delay3*10;
}

//Set Reanidom delay for special rf data
void RF_APP_SetSpecialRFRandomdelay(DWORD MonitorId)
{
    BYTE Slot1 =0, Slot2=0, Slot3=0;
    BYTE buf[17];

    Slot1 = MonitorId % 16;
    Slot2 = MonitorId % 15;
    Slot3 = MonitorId % 14;

    memset(buf, 0, sizeof(buf));

    buf[Slot1] = 1;
    SpecialRFdelay[0] = Slot1;

    Slot2 = GetfreeSlotindex(buf, 16, Slot2);
    buf[Slot2] = 1;
    SpecialRFdelay[1] = Slot2;

    Slot3 = GetfreeSlotindex(buf, 16, Slot3);
    buf[Slot3] = 1;
    SpecialRFdelay[2] = Slot3;

    sort(SpecialRFdelay);
    SetDelayValue(SpecialRFdelay);

}

VOID RF_APP_AverageLBIMeasurement(WORD LBIValue)
{
    DWORD LastAverageValue = sInfo.AverageValue;

    if(LBIValue > 0)
    {
        if(LastAverageValue == 0 && LBIValue > 510)
            AverageValue = LBIValue * 100;
        else
            AverageValue = ((LastAverageValue * 119) + (LBIValue * 100))/120;
	//Removed the LBI Indication as per the OFD Requirement by deepak (task id:10852  IT-374H DIM LEDs)
    }
    sInfo.AverageValue = AverageValue;
}