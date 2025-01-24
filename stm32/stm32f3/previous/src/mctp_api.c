/**
 * @file mctp_api.c
 * @brief MCTP API functions.
 */

/*
 * Channels holds the data to be transmitted as DATA frames. 
 * Channels are independent in data size, allowing different 
 * sample rates for each channel, and data types.
 *
 * The application can create as many channels (up to MAX_CHANNELS),
 * of any size, as long as the total size of channels, which inclu-
 * des both datainfo (DATA_INFO_SIZE bytes for each channel) and
 * buffer sizes, doesn't exceed MAX_DATA_SIZE.
 *
 * Before creating channels, it is necessary to initialize MCTP.
 */

#include "mctp_api.h"
#include "mctp_task.h"

MCTP_Handle *g_Hmctp;

/**
 * @brief Initialize MCTP library and start MCTP communication.
 * @note Ensure the UART associated with the handle passed to hmctp
 *       was initialized before calling this.
 * @note Maximum number of channels is 32.
 * @param hmctp Handle for MCTP communication. Must be configured
 *              before calling this function.
 * @return 0 on success. Negative value if an error occurred.
 *
 * Example
 * -------
 * @code
 * MCTP_Handle hmctp;
 * hmctp.huart = &huart2;
 * hmctp.UserNotifyCallback = mctp_user_callback;
 * hmctp.totalChannels = 8;
 *
 * MCTP_Init(&hmctp);
 * @endcode
 */
int MCTP_Init(MCTP_Handle *hmctp){
    int status = 0;

    if(hmctp->totalChannels > MAX_CHANNELS){
        status = -1;
        goto exit;
    }
    g_Hmctp = hmctp;

    memset(hmctp->recvBuf, 0, RECV_BUFFER_SIZE); 
    hmctp->recvBufIndex = 0;

    memset(&hmctp->channelList, 0, sizeof(MCTP_ChannelList));

    hmctp->state = STATE_IDLE;
    hmctp->userHalt = 0;
    hmctp->userReady = 0;

exit:
    return status;
}

/**
 * @brief Start MCTP communication task.
 * @param hmctp Handle for MCTP communication.
 * @return 0 on success. Negative value if an error occurred.
 */
int MCTP_Start(MCTP_Handle *hmctp){
    hmctp->running = true;
    if(HAL_UART_Receive_IT(hmctp->huart, hmctp->recvBuf, 1) != HAL_OK){
        return -1;
    }
    return 0;
}

/**
 * @brief Stop MCTP communication task.
 * @param hmctp Handle for MCTP communication.
 * @return None
 */
void MCTP_Stop(MCTP_Handle *hmctp){
    hmctp->running = false;
}

/**
 * @brief Notify MCTP communication task.
 * @param hmctp Handle for MCTP communication.
 * @param notif the notification code.
 * @return None
 */
void MCTP_Notify(MCTP_Handle *hmctp, E_MCTP_Notification notif){
    switch(notif){
        case NOTIFY_READY:
            hmctp->userReady = true;
            break;
        case NOTIFY_HALT:
            if(hmctp->state == STATE_TRANS){
                hmctp->userHalt = true;
            }
            break;
        default:
            break;
    }

    MCTP_updateTask(hmctp, EVENT_NOTIF);
}

/**
 * @brief Enable and initialize channel.
 * @param hmctp Handle for MCTP communication.
 * @param channel_id The channel number.
 * @param data_buf Data buffer for the channel.
 * @param buf_size Data buffer size.
 * @param data_type Data type code.
 * @return 0 on success. Negative value if an error occurred.
 */
int MCTP_EnableChannel(MCTP_Handle *hmctp, uint8_t channel_id, uint8_t *data_buf, uint16_t buf_size, E_MCTP_DataType data_type){
    int status = 0;

    if(channel_id >= hmctp->totalChannels){
        status = -1;
        goto exit;
    }
    if(hmctp->channelList.size + buf_size> MAX_DATA_SIZE){
        /* FIXME: channelList size must include dataInfo size */
        status = -1;
        goto exit;
    }
    
    MCTP_Channel *p_channel = &(hmctp->channelList.channels[channel_id]);
    p_channel->dataType = data_type;
    p_channel->dataBuf = data_buf;
    p_channel->bufSize = buf_size;

    hmctp->channelList.size += buf_size;
    if(!hmctp->channelList.map[channel_id]){
        hmctp->channelList.map[channel_id] = 1;
        hmctp->channelList.numberOfChannels += 1;
    }

exit: 
    return status;
}


/**
 * @brief Disable channel.
 * @note This function will not free dynamically allocated memory if
 * user uses it for the channel data buffer.
 * @param hmctp Handle for MCTP communication.
 * @param channel_id The channel number.
 * @return 0 on success. Negative value if an error occurred.
 */
void MCTP_DisableChannel(MCTP_Handle *hmctp, uint8_t channel_id){
    hmctp->channelList.size -= hmctp->channelList.channels[channel_id].bufSize;

    memset(&(hmctp->channelList.channels[channel_id]), 0, sizeof(MCTP_Channel));
    hmctp->channelList.map[channel_id] = 0;
    hmctp->channelList.numberOfChannels -= 1;
}


/**
 * @brief Write data from source to channel buffer.
 * @param hmctp Handle for MCTP communication.
 * @param channel_id The channel number.
 * @param src_buf Data source.
 * @param src_size Number of bytes to be copied from source.
 * @return 0 on success. Negative value if an error occurred.
 */
int MCTP_WriteChannelData(MCTP_Handle *hmctp, uint8_t channel_id, uint8_t *src_buf, uint16_t src_size){
    int status = 0;
    if(src_size > hmctp->channelList.channels[channel_id].bufSize){
        status = -1;
        goto exit;
    }
    if(!hmctp->channelList.map[channel_id]){
        status = -1;
        goto exit;
    }

    /* TODO: error check. Ensure safety before copying */
    uint8_t *channel_data = hmctp->channelList.channels[channel_id].dataBuf;
    memcpy(channel_data, src_buf, src_size);
    hmctp->channelList.channels[channel_id].storedSize = src_size;

exit:
    return status;
}

/**
 * @brief Clear channel data buffer.
 * @param hmctp Handle for MCTP communication.
 * @param channel_id The channel number.
 * @return None
 */
void MCTP_ClearChannelData(MCTP_Handle *hmctp, uint8_t channel_id){
    MCTP_Channel *channel = &hmctp->channelList.channels[channel_id];
    memset(channel->dataBuf, 0, channel->bufSize);
    channel->storedSize = 0;
}

/**
 * @brief Disable and clear all channels.
 * @param hmctp Handle for MCTP communication.
 * @return None
 */
void MCTP_ClearChannelList(MCTP_Handle *hmctp){
    memset(&hmctp->channelList, 0, sizeof(MCTP_ChannelList));
}

/**
 * @brief Send all data from all configured channels. Polling mode.
 * @note All stored data is sent in a single MCTP DATA frame.
 * @param hmctp Handle for MCTP communication.
 * @return 0 on success. Negative value if an error occurred.
 *
 */
int MCTP_SendAll(MCTP_Handle *hmctp){
    int status = 0;

    uint16_t frame_size = 0;
    /* FIXME: Frame size must be dynamic */
    uint8_t frame[2048];    

    if(MCTP_Serialize(hmctp, FRAMETYPE_DATA, frame, 2048, &frame_size) < 0){
        status = -1;
        goto exit;
    }
    HAL_UART_Transmit(hmctp->huart, frame, frame_size, HAL_MAX_DELAY);

exit:
    return status;
}
/* TODO: Interrupt mode. Notify application via txCpltCallback */
