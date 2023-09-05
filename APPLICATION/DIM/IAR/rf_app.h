#ifndef __RF_APP_H__
#define __RF_APP_H__

extern BYTE IAmAliveRetry;
extern BYTE MonitorDataFailCnt;
extern PAGE_RESPONSE response;
extern BYTE PseudoSycnRetryCnt;
extern BYTE triggerDelay;
extern BOOL blnTriggered;
extern BOOL blnTriggerStatus;
extern BYTE RepeatRetry;
extern BYTE IRCount;
extern BYTE LocationDataIndex;
extern DWORD cmdReply;
extern WORD LBIValue;
extern BOOL blnpkttoupgrade;
extern BOOL blnCheckupgradePktreq;

WORD RF_APP_GetBatteryStatus();
VOID RF_APP_SwitchAntenna();
VOID RF_APP_PagingProcess();
BYTE RF_APP_WaitForBeacon(BYTE slot, BYTE offset);
BOOL RF_APP_DoPaging(BOOL bln30System);
BOOL RF_APP_SendMonitorData(BOOL blnNoDelay);

VOID RF_APP_ProcessBCastCommand(BYTE Command);
DWORD RF_APP_ReceiveUpgradeFirmware(DWORD Timeoffset);
VOID RF_APP_OnIRRFHandler(BYTE state);
VOID RF_APP_SendTagData();
VOID RF_APP_GetPackettoUpgrade();
void RF_APP_SetSpecialRFRandomdelay(DWORD MonitorId);
VOID RF_APP_AverageLBIMeasurement(WORD LBIValue);
VOID DimIRRFHandler(BYTE state, BOOL blnPaging);
DWORD RF_APP_ReceiveUpgradeFirmware();

#endif