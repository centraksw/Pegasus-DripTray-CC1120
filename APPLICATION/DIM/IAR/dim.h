#ifndef __DIM_H__
#define __DIM_H__

#define SLAVE_ADDRESS                                 0x13

#define READ_COMMAND                                  0x80
#define IR_LED_CURRENT                                0x83
#define IR_DATA_MSB                                   0x87
#define IR_DATA_LSB                                   0x88
#define MAX_PROXIMITY_DATA                            10

#define TRIGGER_NONE                                  0
#define TRIGGER_MOTION_SENSOR                         1
#define TRIGGER_PROXIMITY_SENSOR                      2
#define TRIGGER_MOTION_OR_PROXIMITY_SENSOR            3
#define TRIGGER_MOTION_AND_PROXIMITY_SENSOR           4


VOID DIM_ProxymityInit();
BOOL DIM_IsProximityTriggered(BYTE Profile);
VOID DIM_ResetProxymityCount();
BOOL DIM_CheckIsTriggered(BYTE Profile);

#endif