#include "defines.h"
#include "general.h"
#include "flash_app.h"
#include "timer_drv.h"

BYTE GetState(BYTE slot)
{
    BYTE tmpslot = (slot % NUM_SLOTS_PER_PC_COM) + 1;

    if( tmpslot == (NUM_SLOTS_PER_PC_COM/2))
        return IR_STATE;
    else if(tmpslot == NUM_SLOTS_PER_PC_COM)
        return PC_COM_REQ_STATE;
    else if(tmpslot == 1)
        return PC_COM_RES_STATE;

    return RF_STATE;
}

BYTE GetBeaconSlot(WORD StarId)
{
    BYTE slot = StarId % 9;
    if(slot == 0) slot = 9;
    if(slot >= 5) slot +=1;
    return slot;
}

BYTE GetSpecialBeaconSlot(WORD StarId)
{
    BYTE slot = GetBeaconSlot(StarId);
    BYTE s1 = StarId / 9;

    if( slot == 10 && s1 > 0)
        s1-=1;
    slot = (slot + s1*12) % MAX_SLOTS;

    return slot;
}

VOID Reset(WORD Counter)
{
   if( Counter == 0 )
   {
      FLASH_APP_Summary_Write();
      WDTCTL=0;
   }
}

BOOL GetBit(WORD pos, BYTE *data)
{
    BOOL blnRetVal = FALSE;

    BYTE byte = pos / 8;
    BYTE bit = pos - (byte * 8);

    blnRetVal = ((data[byte] & (1 << bit)) > 0);

    return blnRetVal;
}

VOID SetBit(WORD pos, BYTE *data)
{
    if( pos > MAX_UPGRADE_SLOTS ) return;

    BYTE byte = pos / 8;
    BYTE bit = pos % 8;

    data[byte] |= (1 << bit);
}

BOOL isBeaconSlotAfterIRState(BYTE slot)
{
    if(slot == 6 || slot == 18 || slot == 30 || slot == 42)
        return TRUE;

    return FALSE;
}

BOOL ForcePaging = FALSE;
BOOL ForceResponse = FALSE;

BOOL IsEthernetPagingSlot(WORD StarId)
{
  BYTE tmpSlot;
  BOOL bPaging = FALSE;

  tmpSlot = (curSlot % NUM_SLOTS_PER_PC_COM);
  if(tmpSlot == 0)
    tmpSlot = NUM_SLOTS_PER_PC_COM;

  if (tmpSlot > 6)
    return 0;

  if (tmpSlot ==  6 && ForcePaging)
    return 1;

  if (tmpSlot == 5 && (StarId % 4) == 0)
    bPaging = TRUE;

  if (tmpSlot == (StarId % 4))
    bPaging = TRUE;

  if (bPaging && (curSlot == GetSpecialBeaconSlot(StarId))) //||
      //(bcast_data.StarType==STAR_REGULAR && (curSlot == LockedBeaconSlot || LockedBeaconSlot ==0) )))
  {
      ForcePaging = 1;
      bPaging = FALSE;
  }

  return bPaging;
}

BOOL IsEthernetResponseRequestSlot(WORD StarId)
{
  BYTE tmpSlot;
  BOOL bRequest = FALSE;

  tmpSlot = (curSlot % NUM_SLOTS_PER_PC_COM);
  if(tmpSlot == 0)
    tmpSlot = NUM_SLOTS_PER_PC_COM;

  if (tmpSlot < 7)
    return 0;

  tmpSlot -= 6;

  if (tmpSlot ==  6 && ForceResponse)
    return 1;

  if (tmpSlot == 5 && (StarId % 4) == 0)
    bRequest = TRUE;

  if (tmpSlot == (StarId % 4))
    bRequest = TRUE;

  if (bRequest && (curSlot == GetSpecialBeaconSlot(StarId))) // ||
      //(bcast_data.StarType==STAR_REGULAR && (curSlot == LockedBeaconSlot || LockedBeaconSlot ==0) )))
  {
      ForceResponse = 1;
      bRequest = FALSE;
  }

  return bRequest;
}

BYTE currState = RF_STATE;
VOID CheckStarRFState(WORD StarId)
{
    if( IsEthernetPagingSlot(StarId))
      currState = PC_COM_REQ_STATE;
    else if( IsEthernetResponseRequestSlot(StarId))
      currState = PC_COM_RES_STATE;
    else
      currState = RF_STATE;

    if( currState == PC_COM_REQ_STATE )
        ForceResponse = FALSE;

    if( currState == PC_COM_RES_STATE )
        ForcePaging = FALSE;

}

BOOL isStarEthernetState(WORD StarId)
{
    if( curSlot == LockedBeaconSlot )
        return TRUE;

    if( IsEthernetPagingSlot(StarId) || IsEthernetResponseRequestSlot(StarId) )
        return TRUE;

    return FALSE;

}
BYTE Get_LFRange(BYTE Profile)
{

    switch(Profile)
    {
    case 0: return LF_RANGE_LOW;
    case 1: return LF_RANGE_MEDIUM;
    case 2: return LF_RANGE_HIGH;
    }
    return LF_RANGE_LOW;
}
