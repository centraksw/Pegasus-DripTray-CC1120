#ifndef __RF_H
#define __RF_H

typedef struct _Frequency {
    BYTE Freq2;
    BYTE Freq1;
    BYTE Freq0;
} FREQUENCY;

typedef struct _BEACON
{
    WORD StarId;
    WORD Command;
    BYTE Info;
    BYTE rssi;
}BEACON;

typedef struct _PageResponse
{
    DWORD DeviceId;
    WORD StarId;
    BYTE Slot;
    BYTE Offset;
    BYTE  Profile1;
    BYTE  Profile2;
    WORD  Profile3;
    DWORD ProfileEx;
}PAGE_RESPONSE;


typedef struct _Ack
{
    DWORD DeviceId;
    WORD cmdValue;
    BYTE Command;
    WORD Command2X;
    BYTE Slot;
    BYTE Offset;
}ACK;


typedef struct _Pkt_LocationData
{
    BYTE Type;
    DWORD TagId;
    WORD RoomId;
    WORD LockedStarId;
    WORD CommandResponse;
    BYTE Status;
    BYTE DataIdx;
    BOOL blnHygieneRoom;
    WORD segtoupgrade;
}PKT_LocationData;

typedef struct Firmware_Info {
    WORD totSeg;
    WORD segIdx;
    BYTE receivedPart;
    BYTE deviceType;
    BYTE UpgradeType;
    BYTE segBuf[MAX_UPGRADE_SEGMENT_SIZE];
    BYTE HardwareVersion;
    BYTE HardwareRevision;
} FIRMWARE_INFO;

BYTE RF_Receive(BYTE* rxBuf, BYTE size, BYTE FT, BYTE HT);

VOID RF_Init(BYTE BitRate);

VOID RF_TurnOn();
VOID RF_TurnOff();

VOID RF_SetFrequencyParams(BYTE freqIdx, BYTE* BeaconChannel, BYTE* DataChannel);

VOID RF_Init_Antenna();
VOID RF_SetAntenna(BYTE Antenna);

VOID RF_SetTXPower(BYTE power);
VOID RF_SetChannel(BYTE Channel);
VOID RF_SendPacket(BYTE *txBuf);
BOOL RF_ReceiveBeacon(BEACON* beacon, BYTE channel, BYTE FT, BYTE HT);

VOID RF_SendPagingRequest(DWORD DeviceId, WORD LBIValue, BYTE Version, BYTE Idx);

BOOL RF_ReceivePagingResponse(DWORD DeviceId, PAGE_RESPONSE* response);
VOID RF_SendPagingRequest3x(DWORD DeviceId, BYTE Part, BYTE Version, BYTE Idx, BYTE MotionFlag, BYTE PsuedoSync );
BOOL RF_ReceivePagingResponse3x(DWORD DeviceId, PAGE_RESPONSE* response);
VOID RF_SendLocationInfo(PKT_LocationData* data);
VOID RF_SendMonitorData3x(BYTE Type, DWORD MonitorId, WORD StarId, DWORD cmdResponse,
                         BYTE Status, BYTE PowerLevel, WORD RoomIds, BYTE blnSendProfile,
                         BYTE Profile1, BYTE Profile2, WORD Profile3, BOOL blnSendSummaryInfo);
VOID RF_SendMonitorData(BYTE Type, DWORD MonitorId, WORD StarId, WORD cmdResponse,  BYTE Status, BYTE PowerLevel, WORD RoomId);
BOOL RF_ReceiveAck(DWORD DeviceId, ACK* ack);

BOOL RF_ReceiveHygieneData(WORD DeviceId, DWORD *TagId);
BOOL RF_ReceiveHygieneData3x(WORD DeviceId, DWORD* TagId);
VOID RF_SendHygieneACK(DWORD TagId);
VOID RF_SendHygieneACK3x(DWORD TagId);

BOOL RF_ReceiveAck3x(DWORD DeviceId, ACK* ack);
VOID RF_SendLocationInfo3x(PKT_LocationData* data);
BOOL RF_ReceiveFirmwareSegment3X(FIRMWARE_INFO* FirmwareInfo);
BOOL RF_ReceivePktUpgrade3X(DWORD DeviceId, FIRMWARE_INFO* FirmwareInfo);
VOID RF_SendPacket2Upgrade(DWORD DeviceId, WORD segtoupgrade, BYTE UpgradeType, BYTE deviceType, BYTE Part, WORD StarId);
BOOL RF_ReceiveFirmwareSegment(FIRMWARE_INFO* FirmwareInfo);
VOID RF_Switch_Bitrate(BYTE Bitrate);
#endif // __RF_H
