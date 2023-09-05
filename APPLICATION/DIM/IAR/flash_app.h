#ifndef __FLASH_APP__
#define __FLASH_APP__

#define ADDR_SUMMARY_INFO_SEGMENT                         0x1040
// Code memory address for TagId, Channel nos and Random delay
#define ADDR_MONITOR_ID                                   0xFD80
#define ADDR_HARDWARE_VERSION                             0xFD84
#define ADDR_HARDWARE_REVISION                            0xFD85
#define ADDR_MONITOR_DELAY                                0xFD90

// Information memory Address for Frequency Index and Sleep state value
#define ADDR_MONITOR_SEGMENT                              0x1000
#define ADDR_MONITOR_FREQ_INDEX                           0x1000

typedef struct __Flash_Settings {

    DWORD DIMId_2X;
    DWORD DIMId_3X;

    WORD ProfileEx;
    BYTE delay[MAX_DELAY_VALUES];
    BYTE FreqIdx;
    BYTE bln30System;
    BYTE blnDIMInSystem;
    BYTE MDMaxCount;
    BYTE TxPower;
    BYTE Threshold;
    BYTE MeasurementRate;
    BYTE SampleCount;
    BYTE MaxProximityValue;
    BYTE calculatingType;

    WORD LBIValue;
    BYTE LBICounter;
    BYTE HardwareVersion;
    BYTE HardwareRevision;
    BYTE Profile;
    BOOL blnLEDBlink;
    WORD RoomId;
    BYTE LFRange;
}FLASH_SETTINGS;

extern FLASH_SETTINGS flash_settings;
VOID FLASH_APP_Read();
VOID FLASH_APP_Write();
VOID FLASH_APP_Summary_Write();
#endif