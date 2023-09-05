#include <string.h>
#include "defines.h"

#include "profile.h"
#include "rf.h"
#include "dfu.h"
#include "flash_drv.h"
#include "general.h"

#define PART1_BASE_ADDR                                 0x2200
#define PART2_BASE_ADDR                                 0x9000
#define INTERRUPT_VECTOR                                0xFF00

#define SEG_SIZE                                        0x200l
#define MAX_UPGRADE_INTERRUPT_SIZE                      48

#define PERCENT_TO_UPGRADE                              5

BYTE FailedToReceiveUpgradePktCnt=0;
FIRMWARE_INFO FirmwareInfo;
BOOL blnEraseSegment = FALSE;
BOOL blnInterruptBufReady = FALSE;
WORD segmentCompleted = 0;

static BYTE part = PART1;
static BYTE UpgradeStatusBits[MAX_UPGRADE_BYTE_SIZE];
static BYTE checkBuf[MAX_UPGRADE_SEGMENT_SIZE];
BYTE InterruptVectorbuf[MAX_UPGRADE_BYTE_SIZE];
VOID DFU_Init()
{
    BYTE r_byte;

    memset(&UpgradeStatusBits, 0, sizeof(UpgradeStatusBits));
    segmentCompleted = 0;
    blnInterruptBufReady = FALSE;
    memset(InterruptVectorbuf,0,sizeof(InterruptVectorbuf));
    //Dectect current part
    FLASH_Read(&r_byte, 0xFFFF, 1);

    if(r_byte >= 0x21 && r_byte <= 0x80 ) part = PART1;
    else part = PART2;
}

WORD DFU_GetUpgradeSlot(WORD StarId)
{
    return (StarId % MAX_TIME_SLOT);
}

VOID DFU_ClearUpgradedBytes(WORD pos)
{
    UpgradeStatusBits[pos] = 0;
    UpgradeStatusBits[pos+1] = 0;
}

WORD DFU_GetUpgradedPacketsCount()
{
    WORD cnt = 0;
    BYTE bit, idx;

    // Check no of bits set in the upgrade bits
    // It will be sent with location data while the upgrade is in progress.
    bit = 0x80;
    while (bit)
    {
        for(idx=0; idx<MAX_UPGRADE_BYTE_SIZE; idx++)
        {
            if( UpgradeStatusBits[idx] & bit )
                cnt++;
        }
        bit >>= 1;
    }
    return cnt;
}

BOOL DFU_CheckIsUpgradeCompleted(WORD TotalSegments)
{
    WORD idx;
    BYTE bytes = TotalSegments / 8;

    // Check all the upgrade bits are set which indicate the upgrade process is completed.
    for(idx=0; idx<bytes; idx++)
    {
        if( UpgradeStatusBits[idx] != 0xFF )
            return FALSE;
    }
    return TRUE;
}

BYTE DFU_GetCurrentWorkingPart()
{
    return part;
}

static BOOL DFU_WriteSegment(FIRMWARE_INFO* FirmwareInfo)
{
    WORD addr;

    BOOL blnRetVal = FALSE, blnCheckBuf = TRUE;
    BYTE idx;

    // calculate the segment to write.f
    if(part == PART1)
        addr = (WORD)PART2_BASE_ADDR;
    else
        addr = (WORD)PART1_BASE_ADDR;

    addr = addr + (FirmwareInfo->segIdx * MAX_UPGRADE_SEGMENT_SIZE);

    // Read the data from the memory.
    FLASH_Read(checkBuf, addr, MAX_UPGRADE_SEGMENT_SIZE);

    // Verify the data with the data to be written if both are same no need to rewrite the data.
    if( memcmp(checkBuf, FirmwareInfo->segBuf, MAX_UPGRADE_SEGMENT_SIZE) == 0 )
        blnRetVal = TRUE;
    else
    {
        // Verify the segment is ready for write (all bytes should be in FF), if not return false to clear the segment.
        for(idx=0; idx<MAX_UPGRADE_SEGMENT_SIZE; idx++)
        {
            if( checkBuf[idx] != 0xFF )
            {
                blnCheckBuf = FALSE;
                break;
            }
        }

        if( blnCheckBuf )
        {
            FLASH_Write(FirmwareInfo->segBuf, addr, MAX_UPGRADE_SEGMENT_SIZE);
            FLASH_Read(checkBuf, addr, MAX_UPGRADE_SEGMENT_SIZE);

            if( memcmp(checkBuf, FirmwareInfo->segBuf, MAX_UPGRADE_SEGMENT_SIZE) == 0 )
                blnRetVal = TRUE;
        }
    }

    return blnRetVal;
}

BOOL DFU_Start(BYTE upgradePart)
{
    if( part != (upgradePart+1) )
        return TRUE;
    return FALSE;
}

VOID DFU_Stop()
{
    memset(&UpgradeStatusBits, 0, sizeof(UpgradeStatusBits));
    settings.blnFirmwareUpgrade = FALSE;
    FailedToReceiveUpgradePktCnt = 0;
    segmentCompleted = 0;
    blnInterruptBufReady = FALSE;
}

VOID DFU_ClearFirmware(WORD segIdx)
{
    WORD seg_addr;

    // Clears 512 byte segment based on the current firmware part.
    if(part == PART1)
        seg_addr = (WORD)PART2_BASE_ADDR;
    else
        seg_addr = (WORD)PART1_BASE_ADDR;

    char* addr = (char*)seg_addr + (segIdx * SEG_SIZE);

    FLASH_Erase_Segment(addr);
}

VOID DFU_WriteFirmware()
{
    BOOL blnRes;

    // Verify the index table, is the segment is already written,
    //If the segment is already written, just exit instead of rewrite the same segment again.
    if( GetBit(FirmwareInfo.segIdx, UpgradeStatusBits) )
        return;

    _DINT();
    if( FirmwareInfo.segIdx == FirmwareInfo.totSeg )
    {
        memcpy(InterruptVectorbuf, FirmwareInfo.segBuf, MAX_UPGRADE_SEGMENT_SIZE);
        blnInterruptBufReady = TRUE;
    }
    else
    {
        // Write the firmware and verify the received firmware data
        // if there is any issue in write or verify process, then erase that segment.
        blnRes = DFU_WriteSegment(&FirmwareInfo);

        // If successfully written, then update the received segments info.
        if( blnRes )
            SetBit(FirmwareInfo.segIdx, UpgradeStatusBits);
        else
            blnEraseSegment = TRUE;
    }
    _EINT();
}

VOID DFU_Write_INTVEC(FIRMWARE_INFO* FirmwareInfo)
{
    char *seg0;
    WORD addr;

    // Where to store the interrupt vector info.
    seg0 = (char *)INTERRUPT_VECTOR;
    addr = INTERRUPT_VECTOR + 0xE0;

    _DINT();
    FLASH_Clear(seg0);

    // Over write the Interrupt vector table.
    FLASH_Write(InterruptVectorbuf, addr, MAX_UPGRADE_SEGMENT_SIZE);

    // Read and verify the interrupt vector table
    FLASH_Read(checkBuf, addr, MAX_UPGRADE_SEGMENT_SIZE);

    Reset(0);
}

BOOL DFU_is5percettoupgrade(WORD segIdx, WORD totseg )
{
    if(!settings.bln30System)
        return FALSE;

    BOOL retval = FALSE;
    WORD PKT_Remaining, PKT_Threshold;

    WORD PktsCnt = DFU_GetUpgradedPacketsCount();
    PKT_Remaining = (totseg - PktsCnt);
    PKT_Threshold = (totseg * PERCENT_TO_UPGRADE) / 100;

    if( PKT_Remaining <= PKT_Threshold )
        retval = TRUE;

    return retval;
}

BOOL DFU_GetSegmenttoupgrade(WORD totSeg, WORD* segIdx)
{
    if( totSeg <= 0 ) return FALSE;

    BOOL retval = FALSE;
    WORD idx;
    BYTE bits;
    WORD segbitcount = 0;

    *segIdx =0;

    WORD bytes = totSeg / 8;
    for(idx=0; idx<bytes; idx++)
    {
        if( UpgradeStatusBits[idx] == 0xFF )
        {
            segbitcount += 8;
            continue;
        }
        for(bits=0; bits<8; bits++)
        {
            if( ((UpgradeStatusBits[idx] >> bits) & 0x1) == 0 )
            {
                *segIdx = (idx*8) + (bits+1);
                return TRUE;
            }
            else
                segbitcount ++;
        }
    }

    if( segbitcount == totSeg )
    {
        *segIdx = (totSeg+1);
        retval = TRUE;
    }

    return retval;
}