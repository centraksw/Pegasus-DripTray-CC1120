#include "defines.h"
#include "general.h"
#include "profile.h"
#include "flash_app.h"

SETTINGS settings;

VOID PROFILE_Init(BYTE Profile1, BYTE Profile2, WORD ProfileEx)
{
    settings.Profile1 = Profile1;
    settings.Profile2 = Profile2;
    settings.ProfileEx = ProfileEx;

    if(settings.bln30System && (settings.Profile != (Profile2 & 0x1F)))
        settings.blnSendProfileToPC = TRUE;

    settings.RoomId = ProfileEx & 0xFFF;
    settings.Profile = Profile2 & 0x1F;
    settings.blnLEDBlink = (Profile2 >> 5) & 0x1;
    settings.LFRange = Get_LFRange((Profile2 >> 6) & 0x3);
    settings.FirmwarePart = (Profile1 >> 3) & 0x1;
    settings.blnFirmwareUpgrade = (Profile1 & 0x80) > 0;

    if( settings.RoomId == 0) settings.RoomId = flash_settings.DIMId_3X & 0xFFF;

    flash_settings.bln30System = settings.bln30System;

    flash_settings.Profile = settings.Profile;
    flash_settings.blnLEDBlink = settings.blnLEDBlink;
    flash_settings.RoomId = settings.RoomId;
    flash_settings.LFRange = settings.LFRange;

    FLASH_APP_Write();
}
WORD PROFILE_GetProfile()
{
    WORD TotalProfile;
    TotalProfile = (settings.Profile1 & 0xF);
    TotalProfile <<= 8;
    TotalProfile |= settings.Profile2;
    return (TotalProfile & 0xFFF);
}

WORD PROFILE_GetVersion()
{
    WORD Profile;
   if(!settings.bln30System)
    {
      Profile = 1;
      Profile <<= 2;
      Profile |= 0;
      Profile <<= 8;
      Profile |= VERSION;
    }
   else
       Profile = VERSION;

    return (Profile & 0xFFF);
}

