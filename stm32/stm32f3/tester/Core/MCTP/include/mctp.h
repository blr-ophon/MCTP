/**
 * @file mctp.h
 * @brief MCTP Fixed macros and struct definitions.
 */
#ifndef MCTP_H
#define MCTP_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#define RECV_BUFFER_SIZE 1024
#define MAX_CHANNELS 32 
#define MAX_DATA_SIZE 65536     /* Maximum represented by 2 bytes */
                          
#define HEADER_SIZE 8
#define DATAINFO_SIZE 4
#define EOM_SIZE 3
#define MIN_FRAME_SIZE (HEADER_SIZE + EOM_SIZE)
#define MAX_FRAME_SIZE (HEADER_SIZE + MAX_DATA_SIZE + EOM_SIZE)
#define SYNCRESP_FRAME_SIZE (HEADER_SIZE + 1 + EOM_SIZE)

/**
 * @enum
 * @brief MCTP communication task state enumeration.
 */
typedef enum{
    STATE_IDLE,
    STATE_SYNC,
    STATE_CONN,
    STATE_TRANS,
} E_MCTP_State;

/**
 * @enum
 * @brief MCTP identifier for data type enumeration.
 */
typedef enum{
    DATATYPE_CHAR     = 0,
    DATATYPE_INT8     = 1,
    DATATYPE_INT16    = 2,
    DATATYPE_INT32    = 3,
    DATATYPE_UINT8    = 4,
    DATATYPE_UINT16   = 5,
    DATATYPE_UINT32   = 6,
    DATATYPE_FLOAT8   = 7,
    DATATYPE_FLOAT16  = 8,
    DATATYPE_FLOAT32  = 9,
} E_MCTP_DataType;

/**
 * @enum
 * @brief MCTP notification types enumeration.
 */
typedef enum{
    SIGNAL_STOP,        /*!< Notify performer of a STOP signal from controller */
    SIGNAL_START,       /*!< Notify performer of a START signal from controller */
    SIGNAL_READY,       /*!< Notify controller of a READY signal from performer */
    SIGNAL_HALT,        /*!< Notify controller of a HALT signal from performer */
} E_MCTP_Signal;

/**
 * @brief MCTP Channel struct definition.
 */
typedef struct{
    uint8_t *dataBuf;           /*!< Buffer that will store channel data */
    uint16_t bufSize;           /*!< Size of dataBuf in bytes */
    int storedSize;             /*!< Size of data stored */
    E_MCTP_DataType dataType;   /*!< Format of data inside dataBuf */
} MCTP_Channel;


/**
 * @brief MCTP Channel list struct definition.
 */
typedef struct{
    MCTP_Channel channels[MAX_CHANNELS];    /*!< All available channels */
    bool map[MAX_CHANNELS];                 /*!< Maps configured channels usage.
                                                Member is set if same indexed channel
                                                in channels array is configured */
    uint8_t numberOfChannels;               /*!< Number of configured channels */
    uint16_t size;                          /*!< Sum of all configured channels
                                                buffers sizes */
} MCTP_ChannelList;

/**
 * @brief MCTP Handle struct definition
 *
 * **Application-defined members** (Must be set by user via MCTP_Init):
 * - 'huart'
 * - 'UserNotifyCallback'
 * - 'totalChannels
 */
typedef struct{
    UART_HandleTypeDef *huart;              /*!< Handle for UART used 
                                                in MCTP communication */
    void ((*SignalCallback)(E_MCTP_Signal));  /*!< Called when mctp 
                                                            task notifies performer */
    uint8_t totalChannels;                  /*!< Enables usage for channels 0 to 
                                               <total_channels> */
    uint8_t recvBuf[RECV_BUFFER_SIZE];      /*!< Buffer for received UART data */
    uint8_t recvBufIndex;                   /*!< Index in buffer for last byte received */
    E_MCTP_State state;                     /*!< Communication task state */
    bool userHalt;                          /*!< Communication task flag. Application 
                                                will stop transmitting DATA frames*/
    bool userReady;                         /*!< Communication task Flag. User ready 
                                                to transmit DATA frames */
    MCTP_ChannelList channelList;           /*!< Channels and channels information */
    bool running;                           /*!< Set during operation */
} MCTP_Handle;

#endif
