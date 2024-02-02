#ifndef _MAIN_H
#define _MAIN_H
#ifdef __2X__
#define VERSION                                             10
#else
#define VERSION                                             4
#endif
extern BOOL blnSendAliveData;
extern BYTE beaconFailCnt;
extern BOOL blnPageReqSent;
extern BOOL blnPagingCompleted;
extern BOOL blnSendSummaryInfo;
extern DWORD AverageValue;
extern BOOL blnBeaconReceived;
extern BOOL blnMonitorWakeup;
extern BYTE ReadyToSendLocationDataCnt;
extern BYTE LockedBeaconSlot;
extern BYTE KeyPressed;

#endif //_MAIN_H
