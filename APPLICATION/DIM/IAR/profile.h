#ifndef __PROFILE_H__
#define __PROFILE_H__

typedef struct _settings {
    BYTE Beacon_Channel;
    BYTE Data_Channel;
    BYTE antenna;
    BYTE Profile;
    BYTE Profile1;
    BYTE Profile2;
    WORD Profile3;
    WORD ProfileEx;
    WORD RoomId ;

    BOOL blnFirmwareUpgrade;
    BYTE FirmwarePart;
    BYTE bln30System;
    BYTE blnSendProfileToPC;
    BOOL blnLEDBlink;
    BYTE LFRange;
}SETTINGS;

typedef struct _Summary_Info {
    DWORD AliveDataWithRetries;
    DWORD AliveDataWithoutRetries;
    DWORD PagingData;
    DWORD LBIMeasurement;
    DWORD SpecialBeaconMissed;
    DWORD TriggerCount;
    DWORD BeaconReceiveCount;
    DWORD WifiData;
    WORD  AverageValue;
}SUMMARY_INFO;

extern SETTINGS settings;
extern SUMMARY_INFO sInfo;

VOID PROFILE_Init(BYTE Profile1, BYTE Profile2, WORD ProfileEx);

WORD PROFILE_GetProfile();
WORD PROFILE_GetVersion();
#endif