#ifndef __DEFINES_H
#define __DEFINES_H

typedef unsigned char                             BOOL;
typedef void		                          VOID;
typedef signed long                               INT32;

typedef unsigned char                             BYTE;
typedef unsigned short                            WORD;
typedef unsigned long                             DWORD;
typedef signed char                               INT8;
typedef signed short                              INT16;

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef NULL
    #define NULL 0
#endif


#define RESET_TIMEOUT                             5000
#define MAX_TIME_SLOT                             288

#define MINUS_20_DBM                              0x43  // This is -11 dbm. Lowest TX power for CC1120 is -11 dbm (Ref: Smart RF Studio)
#define PLUS_3_DBM                                0x64  // +3dbm                         

#define UPGRADE_TIMEGAP_3X                        13000l
#define UPGRADE_TIMEGAP                           20000l
#define MAX_NO_UPGRADE_PKT_RECEIVED_3X            24
#define MAX_NO_UPGRADE_PKT_RECEIVED               10


#define BRATE_76_KBPS                             0
#define BRATE_50_KBPS                             1
#define BRATE_150_KBPS                            2

#define TIME_SLOT                                 250
#define TIME_SLOT_US                              250000

#define SLEEP_FOR_BEACON_TX                       120000l
////////////////////////////////////////////////////////////////////////////////

#define PAGE_DELAY_1_5_SEC                        6
#define PAGE_DELAY_250_MS                         1
#define PAGE_DELAY_12_5_SEC                       50
#define PAGE_DELAY_9_5_SEC                        38
#define PAGE_DELAY_6_SECS                         24



#define LOW_BATTERY_CYCLE_COUNTER_3X              14400    // 10 Mins  (10 * 60 * 4)
#define LOW_BATTERY_CYCLE_COUNTER                 2400    // 10 Mins  (10 * 60 * 4)
#define SUMMARY_WRITE_COUNTER                     172800    // 12 hours

//////////////////////////////////////////////////////////////////////////////
////// BITRATE_76_8_KBPS
//////////////////////////////////////////////////////////////////////////////
#define BEACON_RECEIVE_TIME                       6800        //5400
#define TAG_FLYOVER_TIME                          30

#define MONITOR_I_AM_ALIVE_INTERVAL               14400       // 1 Hour. (60 * 60 * 4)
#define MONITOR_SUMMARY_INTERVAL                  345600      //24 hours

#define MAX_DELAY_VALUES                          16

#define MAX_MONITOR_INFO_RETRY                     3
#define MAX_MONITOR_BEACON_FAIL                    6
#define MAX_MONITOR_DATA_FAIL                      2


#define MAX_MONITOR_PSEUDO_SYNC_FAIL               3
#define MAX_MONITOR_PSEUDO_SYNC                    2

#ifdef __2X__
#define MAX_FREQUENCY                             13
#else
#define MAX_FREQUENCY                             15
#endif
#define NUM_SLOTS_PER_PC_COM                      12

#define MAX_SLOTS                                 48

#define RF_STATE                                  1
#define IR_STATE                                  2
#define PC_COM_REQ_STATE                          3
#define PC_COM_RES_STATE                          4


#define RF_HDR_PAGING_REQUEST_EX                  6
#define RF_HDR_PAGING_RESPONSE_EX                 7
#define RF_HDR_TAG_INFO_EX                        8
#define RF_HDR_ACK_EX                             9
#define RF_HDR_MONITOR_INFO_EX                    10
#define RF_HDR_FW_PACKET                          12
#define RF_HDR_HYGIENE_DATA                       14
#define RF_HDR_HYGIENE_ACK                        15

#define RF_HDR_TAG_INFO_3X_TYPE8                  39

#define RF_HDR_PAGING_REQUEST_3X                  16
#define RF_HDR_PAGING_RESPONSE_3X                 17
#define RF_HDR_ACK_3X                             25
#define RF_HDR_MONITOR_INFO_3X_TYPE1              26
#define RF_HDR_MONITOR_INFO_3X_TYPE2              27
#define RF_HDR_MONITOR_INFO_3X_TYPE3              52

#define RF_HDR_HYGIENE_DATA_3X                    38


#define MONITOR_INFO_LEN_3X_TYPE1                  14
#define MONITOR_INFO_LEN_3X_TYPE2                  30
#define MONITOR_INFO_LEN_3X_TYPE3                  40

#define MAX_PKT_LEN                                44

#define MAX_UPGRADE_SLOTS                         896
#define MAX_UPGRADE_BYTE_SIZE                     112
#define FW_PKT_LEN                                37
#define FW_PKT_LEN_EX                             39

#define MAX_UPGRADE_SEGMENT_SIZE                  32
#define UPGRADE_PKT_PER_CYCLE_2X                  4
#define UPGRADE_PKT_PER_CYCLE_3X                  8
#define FW_FINISH_CMD_LEN                         34
//#define FW_PKT_LEN_EX                           38
#define FWR_PKTUPGRADE_LEN_3X                     42
#define MONITOR_INFO_LEN_3X_PKTUPGRADE            12
#define RF_HDR_MONITOR_INFO_3X_PKTUPGRADE         54
#define RF_HDR_FW_PKTUPGRADE                      55
#define DIM_UPGRADE_PACKET_TIMEOUT                480//1200

#define BEACON_PKT_LEN                            4
#define STAR_ACK_PKT_LEN                          6
#define PAGE_REQ_PKT_LEN                          7
#define PAGE_RES_PKT_LEN                          9
#define TAG_INFO_LEN                              9
#define MONITOR_INFO_LEN                          7


#define PAGE_RES_PKT_LEN_3X                       25
#define PAGE_REQ_PKT_LEN_3X                       11
#define STAR_ACK_PKT_LEN_3X                       10
#define RF_HDR_MINIMAL_ACK_3X                     57
#define STAR_MINIMAL_ACK_PKT_LEN_3X               7

#define STAR_ACK_PKT_LEN_EX                       8
#define PAGE_REQ_PKT_LEN_EX                       7
#define PAGE_RES_PKT_LEN_EX                       12
#define TAG_INFO_LEN_EX                           11
#define TAG_INFO_LEN_EX_3X                        8
#define MONITOR_INFO_LEN_EX                       11



#define HYGIENE_DATA_PKT_LEN                      6
#define HYGIENE_DATA_PKT_LEN_3X                   7
#define HYGIENE_ACK_PKT_LEN                       4
#define HYGIENE_ACK_PKT_LEN_3X                    5



#define TAG_HYGIENE                               1
#define TAG_HYGIENE_3x                            8


#define MONITOR_DIM                               1

#define MONITOR_DRIP_TRAY_DIM_3X                  18

#ifdef __EXTERNAL_DIM__
#define MONITOR_DIM_3X                            17 
#define MONITOR_TYPE                              MONITOR_DIM_3X
#else
#define MONITOR_TYPE                              MONITOR_DRIP_TRAY_DIM_3X
#endif
#define MONITOR_UPGRADE                           1


#define CMD_MONITOR_RESET		                  0x1
#define CMD_MONITOR_SET_FREQUENCY	              0x3
#define CMD_MONITOR_GET_PROFILE		              0x5
#define CMD_MONITOR_GET_VERSION		              0x6
#define CMD_MONITOR_GET_BATTERY_STATUS	          0x7
#define CMD_MONITOR_UPGRADE_INFO		          0xC
#define CMD_MONITOR_CLEAR_SUMMARY                 0x12
#define CMD_MONITOR_GET_SUMMARY_INFO              0x13

#define BCAST_CMD_WAKEUP	                      0x3

#define LED_ON                                    1
#define LED_ON_TIME                               8    // 2 msec   8*8*30.51 = 1952.64 us

#define PART1                                     1
#define PART2                                     2

#define DEVICE_ID_MASK_3X                         0x3FFFFFFF
#define DEVICE_ID_MASK                            0x3FFFFF

#define MONITOR_BASE_ID_3X                        0xC0000000
#define MONITOR_BASE_ID                           0xC00000


#define LF_RANGE_LOW                             5
#define LF_RANGE_MEDIUM                          6
#define LF_RANGE_HIGH                           16

#include "msp430g2955.h"
#include "pinconfig.h"
#include "main.h"

#endif // __DEFINES_H
