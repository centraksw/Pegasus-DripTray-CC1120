#ifndef _DFU_H
#define _DFU_H

extern BYTE FailedToReceiveUpgradePktCnt;
extern FIRMWARE_INFO FirmwareInfo;
extern BOOL blnEraseSegment;
extern BOOL blnInterruptBufReady;
extern WORD segmentCompleted;

VOID DFU_Init();
WORD DFU_GetUpgradeSlot(WORD StarId);
BOOL DFU_CheckIsUpgradeCompleted(WORD TotalSegments);
WORD DFU_GetUpgradedPacketsCount();
VOID DFU_ClearUpgradedBytes(WORD pos);

BOOL DFU_Start(BYTE upgradePart);
VOID DFU_Stop();
BYTE DFU_GetCurrentWorkingPart();

VOID DFU_ClearFirmware(WORD segIdx);
VOID DFU_WriteFirmware();
VOID DFU_Write_INTVEC(FIRMWARE_INFO* FirmwareInfo);
BOOL DFU_is5percettoupgrade(WORD segIdx, WORD totseg );
BOOL DFU_GetSegmenttoupgrade(WORD totSeg, WORD* segIdx);

#endif //_DFU_H
