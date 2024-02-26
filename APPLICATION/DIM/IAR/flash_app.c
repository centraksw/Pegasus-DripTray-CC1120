#include <string.h>
#include "defines.h"
#include "flash_app.h"
#include "flash_drv.h"
#include "dim.h"
#include "profile.h"
#include "general.h"

FLASH_SETTINGS flash_settings;

VOID FLASH_APP_Write()
{
    BYTE buf[15];
    char* Addr = (char*) ADDR_MONITOR_SEGMENT;

    buf[0] = flash_settings.FreqIdx;
    buf[1] = flash_settings.bln30System;
    buf[2] = flash_settings.blnDIMInSystem;
    buf[3] = flash_settings.MDMaxCount;
    buf[4] = flash_settings.TxPower;
    buf[5] = flash_settings.Threshold;
    buf[6] = flash_settings.MeasurementRate;
    buf[7] = flash_settings.SampleCount;
    buf[8] = flash_settings.MaxProximityValue;
    buf[9] = flash_settings.calculatingType;
    buf[10] = flash_settings.Profile;
    buf[11] = flash_settings.blnLEDBlink;

    memcpy(buf+12, &flash_settings.RoomId, 2);
    buf[14] = flash_settings.LFRange;

    // Clear the whole segment and write in one shot
    _DINT();
    FLASH_Clear(Addr);
    FLASH_Write(buf, ADDR_MONITOR_SEGMENT, 15);
    _EINT();
}

VOID FLASH_APP_Summary_Write()
{
    char* Addr = (char*) ADDR_SUMMARY_INFO_SEGMENT;
    // Clear the whole segment and write in one shot
    _DINT();
    FLASH_Clear(Addr);
    FLASH_Write(&sInfo, ADDR_SUMMARY_INFO_SEGMENT, sizeof(sInfo));
    _EINT();
}

VOID FLASH_Summary_Read()
{
    memset(&sInfo, 0xAA, sizeof(sInfo));
    memset(&sInfo, 0, sizeof(sInfo));
    FLASH_Read(&sInfo, ADDR_SUMMARY_INFO_SEGMENT, sizeof(sInfo));
    if(sInfo.AliveDataWithRetries >= 0xFFFFFF)sInfo.AliveDataWithRetries = 0;
    if(sInfo.AliveDataWithoutRetries >= 0xFFFFFF)sInfo.AliveDataWithoutRetries = 0;
    if(sInfo.PagingData >= 0xFFFFFF)sInfo.PagingData = 0;
    if(sInfo.LBIMeasurement >= 0xFFFFFF)sInfo.LBIMeasurement = 0;
    if(sInfo.TriggerCount >= 0xFFFFFF)sInfo.TriggerCount = 0;
    if(sInfo.BeaconReceiveCount >= 0xFFFFFF)sInfo.BeaconReceiveCount = 0;
    if( sInfo.AverageValue == 0xFFFF ) sInfo.AverageValue = 0;
}

VOID FLASH_APP_Read()
{
    BYTE buf[15];
#ifdef __DEBUG_MODE__
    flash_settings.DIMId_3X = 0xC0000000 + 2112;
    flash_settings.FreqIdx = 5;
    BYTE delay[MAX_DELAY_VALUES] = {160, 140, 200, 130, 160, 170, 200, 190, 160, 200, 180, 170, 160, 210, 180, 130};

    flash_settings.MDMaxCount = 10;
    flash_settings.TxPower = 20;
    flash_settings.Threshold = 10;
    flash_settings.MeasurementRate = 1;
    flash_settings.SampleCount = 1;
    flash_settings.MaxProximityValue = 10;
    flash_settings.calculatingType = 1;

    FLASH_APP_Write();


    FLASH_Write(&flash_settings.DIMId_3X, ADDR_MONITOR_ID, sizeof(flash_settings.DIMId_3X) );
    FLASH_Write(&flash_settings.FreqIdx,  ADDR_MONITOR_FREQ_INDEX, sizeof(flash_settings.FreqIdx) );

    flash_settings.HardwareVersion = 1;
    flash_settings.HardwareRevision = 1;

    FLASH_Write(&flash_settings.HardwareVersion, ADDR_HARDWARE_VERSION, sizeof(flash_settings.HardwareVersion) );
    FLASH_Write(&flash_settings.HardwareRevision, ADDR_HARDWARE_REVISION, sizeof(flash_settings.HardwareRevision) );

    FLASH_Write(&delay, ADDR_MONITOR_DELAY, sizeof(delay) );
#endif

    flash_settings.DIMId_3X = 0;
    // Read TagId from code memory.
    FLASH_Read(&flash_settings.DIMId_3X, ADDR_MONITOR_ID, sizeof(flash_settings.DIMId_3X));
    flash_settings.DIMId_2X = ( flash_settings.DIMId_3X & DEVICE_ID_MASK ) | MONITOR_BASE_ID;

    // Read Randomd Delay values from code memory.
    FLASH_Read(&flash_settings.delay, ADDR_MONITOR_DELAY, sizeof(flash_settings.delay) );

    FLASH_Read(buf, ADDR_MONITOR_SEGMENT, 15);

    // Read Frequency Index from Flash.
    flash_settings.FreqIdx = buf[0];
    if( flash_settings.FreqIdx < 1 || flash_settings.FreqIdx > MAX_FREQUENCY )
        flash_settings.FreqIdx = 1;

#if 0 
    flash_settings.bln30System = buf[1];
    if(flash_settings.bln30System == 0xFF)
        flash_settings.bln30System = FALSE;

    flash_settings.blnDIMInSystem = buf[2];
    if(flash_settings.blnDIMInSystem == 0xFF)
        flash_settings.blnDIMInSystem = FALSE;
#endif
    
    flash_settings.blnDIMInSystem = TRUE;
#ifdef __2X__
    flash_settings.bln30System = FALSE;
#else
    flash_settings.bln30System = TRUE;	
#endif
    flash_settings.MDMaxCount = buf[3];
    flash_settings.TxPower = buf[4];
    if( flash_settings.TxPower > 20 ) flash_settings.TxPower = 20;

    flash_settings.Threshold = buf[5];

    flash_settings.MeasurementRate = buf[6];
    if( flash_settings.MeasurementRate == 0 ) flash_settings.MeasurementRate = 1;

    flash_settings.SampleCount = buf[7];
    if( flash_settings.SampleCount == 0 ) flash_settings.SampleCount = 1;

    flash_settings.MaxProximityValue = buf[8];
    if( flash_settings.MaxProximityValue == 0 || flash_settings.MaxProximityValue > MAX_PROXIMITY_DATA ) flash_settings.MaxProximityValue = MAX_PROXIMITY_DATA;

    flash_settings.calculatingType = buf[9];

    flash_settings.Profile = buf[10];
    if( flash_settings.Profile == 0xFF ) flash_settings.Profile = 0;
    settings.Profile = flash_settings.Profile;
    settings.bln30System=flash_settings.bln30System;

    flash_settings.blnLEDBlink = buf[11];
    if( flash_settings.blnLEDBlink == 0xFF ) flash_settings.blnLEDBlink = 1;
    settings.blnLEDBlink=flash_settings.blnLEDBlink;

    memcpy(&flash_settings.RoomId, buf+12, 2);
    if( flash_settings.RoomId == 0xFFFF )
        flash_settings.RoomId = flash_settings.DIMId_3X & 0xFFF;
    settings.RoomId=flash_settings.RoomId;

    flash_settings.LFRange = buf[14];
    if( flash_settings.LFRange == 0xFF)  flash_settings.LFRange = 0;
    settings.LFRange=flash_settings.LFRange;

    FLASH_Read(&flash_settings.HardwareVersion, ADDR_HARDWARE_VERSION, sizeof(flash_settings.HardwareVersion) );
    FLASH_Read(&flash_settings.HardwareRevision, ADDR_HARDWARE_REVISION, sizeof(flash_settings.HardwareRevision) );

    FLASH_Summary_Read();
	
	//Set constant values for Measurement Rate and Threshold	
    flash_settings.MeasurementRate = 1;	
    flash_settings.Threshold = 80;
}
