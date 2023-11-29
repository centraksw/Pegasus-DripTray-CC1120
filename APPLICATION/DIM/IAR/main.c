#include <string.h>
#include "defines.h"
#include "profile.h"
#include "ccxx2x_drv.h"
#include "rf.h"
#include "timer_drv.h"
#include "rf_spi_drv.h"
#include "io.h"
#include "general.h"
#include "flash_app.h"
#include "dfu.h"
#include "rf_app.h"
#include "motion.h"
#include "dim.h"
#include "lf.h"

BYTE beaconFailCnt = 0;
BOOL blnPagingCompleted = FALSE;
BOOL blnPageReqSent = FALSE;
BOOL blnSendAliveData = FALSE;
BOOL blnSendSummaryInfo = FALSE;
DWORD AverageValue = 0;

BOOL blnBeaconReceived = FALSE;
static BEACON beacon;
BYTE LockedBeaconSlot = 0;
static DWORD SleepTimeToUpgrade;
static DWORD offset;
BYTE ReadyToSendLocationDataCnt = 0;
BOOL blnMonitorWakeup;
BYTE KeyPressed = 0;

VOID main()
{
    BYTE state, idx;

    LF_Initialize();

    DBG_INIT_MONITOR();

    // Initialize the Micro controller
    TIMER_MicroInit();

    // Initalize the Timer
    TIMER_Init();

    MD_Init();

    // Initalize the SPI
    RF_SPI_DRV_Init();

#ifdef __2X__
    RF_Init(BRATE_76_KBPS);
#else
	RF_Init(BRATE_50_KBPS);
#endif 
    manualCalibration();

    // Setup the Antennas
    RF_Init_Antenna();

    DFU_Init();

    // Read settings from flash memory
    FLASH_APP_Read();

    RF_SetFrequencyParams(flash_settings.FreqIdx, &settings.Beacon_Channel, &settings.Data_Channel);

    IO_TurnOffLED(LED_YELLOW);
    DIM_ProxymityInit();

    // Disable all unsued IO Pins.
    IO_ShortSleep_Enter();

    // Read battery voltage.
    LBIValue = RF_APP_GetBatteryStatus();
    RF_APP_AverageLBIMeasurement(LBIValue);

    ReadyToSendLocationDataCnt = 0;

    while( TRUE )
    {
        // If paging is not completed, then go to paiging sequence.
        if( !blnPagingCompleted )
        {
            LocationDataIndex = 0;
            beaconFailCnt = 0;
            cmdReply = 0;
            if( settings.blnFirmwareUpgrade )
                DFU_Init();

            RF_APP_PagingProcess();
            g_LocationCounter = MONITOR_I_AM_ALIVE_INTERVAL;
            settings.blnSendProfileToPC = TRUE;
            flash_settings.blnDIMInSystem = TRUE;
        }

        blnPagingCompleted = FALSE;
        blnPageReqSent = FALSE;

        curSlot = RF_APP_WaitForBeacon(response.Slot, response.Offset);
        PROFILE_Init(response.Profile1, response.Profile2, response.Profile3);
        LockedBeaconSlot = GetSpecialBeaconSlot(response.StarId);

        if( settings.blnFirmwareUpgrade)
        {
            settings.blnFirmwareUpgrade = DFU_Start(settings.FirmwarePart);
            g_UpgradeLocationCounter = DIM_UPGRADE_PACKET_TIMEOUT;
        }

        settings.antenna = RF_ANTENNA1;
        blnBeaconReceived = FALSE;
        beaconFailCnt = 0;
        if(settings.bln30System)
            g_LocationCounter = (flash_settings.DIMId_3X % 480);
        else
            g_LocationCounter = (flash_settings.DIMId_2X % 480);

        blnMonitorWakeup = TRUE;
        MonitorDataFailCnt = 0;
        PseudoSycnRetryCnt = 0;

        blnTriggered = FALSE;
        blnTriggerStatus = FALSE;
        triggerDelay = 0;
        RepeatRetry = 0;
        DIM_ResetProxymityCount();

        while (TRUE)
        {
            if( IO_ScanKeys() )
            {
                KeyPressed = 1;
                blnSendAliveData = TRUE;
                g_LocationCounter = MONITOR_I_AM_ALIVE_INTERVAL;
            }

            if( ReadyToSendLocationDataCnt > 0 )
                ReadyToSendLocationDataCnt -= 1;
            if( DIM_CheckIsTriggered(settings.Profile) && IRCount==0 && triggerDelay==0 )
            {
                IRCount = 1000;
                blnTriggered = TRUE;
                blnTriggerStatus = TRUE;
                sInfo.TriggerCount += 1;
                g_LocationCounter = MONITOR_I_AM_ALIVE_INTERVAL;
            }

            //Randomization fix
            CheckStarRFState(response.StarId);

            state = GetState(curSlot) ;
            switch( state )
            {
            case RF_STATE:
                // If beacon received or current slot is not locked beacon slot,
                // use the current slot to send Monitor data
                if( blnBeaconReceived || (curSlot != LockedBeaconSlot) )
                {
                    RF_APP_OnIRRFHandler( state );
                    break;
                }

                offset = 0;
                SleepTimeToUpgrade = 0;

                if(settings.bln30System)
                {
                    if(settings.blnFirmwareUpgrade && DFU_GetUpgradeSlot(response.StarId) > 144 && !isBeaconSlotAfterIRState(LockedBeaconSlot))
                        offset = RF_APP_ReceiveUpgradeFirmware(offset);

                    TIMER_DelayUS(SLEEP_FOR_BEACON_TX - offset);

                    offset = SLEEP_FOR_BEACON_TX;
                }

                // Initialize slot offset to calculate how much time we spent to receive beacon.
                TIMER_InitSlotOffset();

                // Set RF Antenna
                RF_SetAntenna( settings.antenna );

                // Disable motion sensor
                MD_Disable();

                // Receive beacon.  Beacon timeout will be increased based on beacon fail count.
                blnBeaconReceived = RF_ReceiveBeacon(&beacon, settings.Beacon_Channel, beaconFailCnt + 7, beaconFailCnt + 15);

                // Enable motion Sensor.
                MD_Enable();

                // If Beacon received,
                if(blnBeaconReceived)
                {
                    sInfo.BeaconReceiveCount += 1;
                    beaconFailCnt = 0;
                    RF_APP_ProcessBCastCommand(beacon.Command);
                    offset += BEACON_RECEIVE_TIME;
                }
                else
                {
                    RF_APP_SwitchAntenna();

                    // Increment Beacon Fail count
                    beaconFailCnt += 1;

                    // Set Beacon received as TRUE to prevent re enter into this state.
                    blnBeaconReceived = TRUE;

                    // Calcuate time spent in beacon receive process.
                    offset += TIMER_GetSlotOffsetInUS();
                }

                if(!settings.bln30System)
                {
                    if( settings.blnFirmwareUpgrade  )
                    {
                        if( DFU_GetUpgradeSlot(response.StarId) > 144 )
                            SleepTimeToUpgrade = 17000l;
                        else
                            SleepTimeToUpgrade = 137000l;

                        if( offset < SleepTimeToUpgrade )
                            TIMER_DelayUS(SleepTimeToUpgrade - offset);

                        offset = RF_APP_ReceiveUpgradeFirmware(offset);

                        offset += SleepTimeToUpgrade;

                        blnSendAliveData = TRUE;
                        g_LocationCounter = MONITOR_I_AM_ALIVE_INTERVAL;
                    }
                }

                if( settings.bln30System && settings.blnFirmwareUpgrade)
                {
                    if((DFU_GetUpgradeSlot(response.StarId) <= 144 || isBeaconSlotAfterIRState(LockedBeaconSlot)))
                        offset += RF_APP_ReceiveUpgradeFirmware(offset);

                    if(g_LocationCounter >= DIM_UPGRADE_PACKET_TIMEOUT || beaconFailCnt > 0 )
                    {
                        blnSendAliveData = TRUE;
                        g_LocationCounter = 0;
                        ReadyToSendLocationDataCnt = (flash_settings.DIMId_3X % 5) + 2;
                    }
                }

                // Stop WDT
                TIMER_StopWDT();

                // Disable unused IO Pins.
                IO_ShortSleep_Enter();

                // Sleep  in LPM1 ( remaining time of the current slot )
                TIMER_DelayUS( TIME_SLOT_US - offset );

                // Enables the required IO Pins.
                IO_ShortSleep_Exit();

                //Set Current Slot.
                curSlot = LockedBeaconSlot + 1;

                //Start the WDT. Now WDT will be synced with Star WDT.
                TIMER_InitWDT();
                continue;

            case IR_STATE:
                // Initialize Spider data Retry count to 1
                IAmAliveRetry = 1;
                break;

            case PC_COM_RES_STATE:
                RF_APP_OnIRRFHandler(state);

                if( settings.bln30System && settings.blnFirmwareUpgrade && blnCheckupgradePktreq )
                {
                    if(DFU_is5percettoupgrade(FirmwareInfo.segIdx, FirmwareInfo.totSeg ))
                        blnpkttoupgrade = TRUE;
                }

                if( blnEraseSegment )
                {
                    blnEraseSegment = FALSE;

                    idx = FirmwareInfo.segIdx / 16;
                    DFU_ClearFirmware(idx);

                    idx = idx*2;
                    DFU_ClearUpgradedBytes(idx);
                }
                else
                {
                    if( blnInterruptBufReady )
                    {
                        if( DFU_CheckIsUpgradeCompleted(FirmwareInfo.totSeg) )
                            DFU_Write_INTVEC(&FirmwareInfo);
                    }
                }

                // Check battery status once in every 10 mins.
                if(settings.bln30System)
                {
                    if( (g_LocationCounter >= MONITOR_I_AM_ALIVE_INTERVAL) || blnMonitorWakeup )
                    {
                        ReadyToSendLocationDataCnt = (flash_settings.DIMId_3X % 5) + 2;
                        blnSendAliveData = TRUE;
                    }
                    if( g_BatteryCounter > LOW_BATTERY_CYCLE_COUNTER_3X )
                    {
                        sInfo.LBIMeasurement += 1;
                        LBIValue = RF_APP_GetBatteryStatus();
                        RF_APP_AverageLBIMeasurement(LBIValue);
                    }

                    if( g_SummaryCounter > MONITOR_SUMMARY_INTERVAL)
                    {
                        g_SummaryCounter = 0;
                        blnSendSummaryInfo = TRUE;
                        blnSendAliveData = TRUE;
                        g_LocationCounter  = MONITOR_I_AM_ALIVE_INTERVAL;
                    }

                    if( g_SummaryWriteCounter > SUMMARY_WRITE_COUNTER )
                    {
                        g_SummaryWriteCounter = 0;
                        FLASH_APP_Summary_Write();
                    }
                }
                else
                {
                    if( (g_LocationCounter >= MONITOR_I_AM_ALIVE_INTERVAL) || blnMonitorWakeup )
                    {
                        ReadyToSendLocationDataCnt = (flash_settings.DIMId_2X % 5) + 2;
                        blnSendAliveData = TRUE;
                    }
                    if( g_BatteryCounter > LOW_BATTERY_CYCLE_COUNTER  )
                    {
                        LBIValue = RF_APP_GetBatteryStatus();
                        RF_APP_AverageLBIMeasurement(LBIValue);
                    }
                }

                break;
            }
            // If no beacon for 6 cycles or no pseudo sync response for 3 times
            // Exit from main loop and do regular paging.
            if( beaconFailCnt >= MAX_MONITOR_BEACON_FAIL || PseudoSycnRetryCnt >= MAX_MONITOR_PSEUDO_SYNC_FAIL )
            {
                // Stops slot timer
                TIMER_StopWDT();
                break;
            }

            if( triggerDelay > 0 )
                triggerDelay -= 1;

            if( (IRCount == 0) && (triggerDelay==0) )
                DIM_IsProximityTriggered(settings.Profile);


            // Disable all unused IO pins
            IO_ShortSleep_Enter();

            // Put the micro in LPM3 mode
            Micro_Sleep();

            // Enable required IO pins.
            IO_ShortSleep_Exit();

            // Check is this last slot of current cycle
            if( curSlot == MAX_SLOTS )
            {
                // Reset Beacon Received flag.
                blnBeaconReceived = FALSE;

                // Reset Page request sent flag
                blnPageReqSent = FALSE;
            }
        }
    }
}
