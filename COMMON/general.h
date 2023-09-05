#ifndef __GENERAL_H
#define __GENERAL_H

BYTE GetState(BYTE slot);
BYTE GetBeaconSlot(WORD StarId);
BYTE GetSpecialBeaconSlot(WORD StarId);
VOID Reset(WORD counter);
BOOL GetBit(WORD pos, BYTE *data);
VOID SetBit(WORD pos, BYTE *data);
BOOL isBeaconSlotAfterIRState(BYTE slot);
BOOL isStarEthernetState(WORD StarId);
VOID CheckStarRFState(WORD StarId);
BYTE Get_LFRange(BYTE Profile);
#endif

