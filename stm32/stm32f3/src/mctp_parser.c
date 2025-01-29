/*
 * FRAME
 * *------------*----------*-----*
 * | HEADER (8) | DATA (x) | EOM |
 * *------------*----------*-----*
 * 
 * >HEADER section
 * *-------------*--------------*-------------*
 * | FRM_TYPE(1) | DATA_SIZE(2) | RESERVED(5) |
 * *-------------*--------------*-------------*
 * 
 * >DATA section (DATA frame)
 * *--------------*---------------*-----------------*----------------*------------*-----*
 * | SEQUENCE (1) | CHANNEL_ID(1) | SAMPLES_SIZE(2) | DATA_FORMAT(1) | SAMPLES(x) | ... |
 * *--------------*---------------*-----------------*----------------*------------*-----*
 * *-----*---------------*-----------------*----------------*------------*-----*
 * | ... | CHANNEL_ID(1) | SAMPLES_SIZE(2) | DATA_FORMAT(1) | SAMPLES(x) | ... |
 * *-----*---------------*-----------------*----------------*------------*-----*
 * (*) TODO: SEQUENCE bits currently not implemented
 * 
 * >DATA section (SYNC RESP frame)
 * *------------------*
 * | N_OF_CHANNELS(1) |
 * *------------------*
 * 
 * >EFD
 * *------------*
 * |0x242526 (3)|
 * *------------*
 */


#include "mctp_parser.h"


/*
 * Creates serialized frame based on <frame_type>, stores it on
 * <frame_buf> and it's size on <frame_size>.
 * In case of frames with data section, such as SYNC_RESP and DATA,
 * data_section is generated based on available channels on <hmctp>
 * channel list. 
 */
int MCTP_Serialize(MCTP_Handle *hmctp, E_MCTP_FrameType frame_type, uint8_t *frame_buf, int frame_buf_size, uint16_t *frame_size){
    int status = 0;

    if(frame_buf_size < HEADER_SIZE + EOM_SIZE){
        status = -1;
        goto exit;
    }

    uint16_t total_data_size = 0;
    uint8_t *p_data_section = frame_buf + HEADER_SIZE;;

    /* 
     * DATA Section
     */
    switch(frame_type){
        case FRAMETYPE_SYNC_RESP:
            {
            /* N of channels */
            memcpy(p_data_section, &(hmctp->totalChannels), 1);
            p_data_section+= 1;
            total_data_size += 1;
            }
            break;
        case FRAMETYPE_DATA:
            {
            /* N of channels */
            memcpy(p_data_section, &(hmctp->channelList.numberOfChannels), 1);
            p_data_section += 1;
            total_data_size += 1;

            /* Datainfo + Data */
            for(int i = 0; i < MAX_CHANNELS; i++){
                if(hmctp->channelList.map[i] && hmctp->channelList.channels[i].storedSize > 0){
                    uint8_t *data = hmctp->channelList.channels[i].dataBuf;
                    uint16_t data_size = hmctp->channelList.channels[i].storedSize;
                    E_MCTP_DataType data_type = hmctp->channelList.channels[i].dataType;

                    total_data_size += data_size + DATAINFO_SIZE;
                    if(total_data_size > MAX_DATA_SIZE){
                        /* FIXME: Irrelevant, since total_data_size is 2 bytes */
                        status = -1;
                        goto exit;
                    }
                    /* Channel id */
                    memcpy(p_data_section, &i, 1);  
                    p_data_section += 1;
                    /* Data size */
                    memcpy(p_data_section, &data_size, 2);
                    p_data_section += 2;
                    /* Data type */
                    memcpy(p_data_section, &data_type, 1);
                    p_data_section += 1;
                    memcpy(p_data_section, data, data_size);
                    p_data_section += data_size;
                }
            }
            }
            break;
        case FRAMETYPE_SYNC:
            break;
        case FRAMETYPE_ACK:
            break;
        case FRAMETYPE_REQUEST:
            break;
        case FRAMETYPE_STOP:
            break;
        case FRAMETYPE_DROP:
            break;
        default:
            status = -1;
            goto exit;
    }

    /* HEADER Section */
    frame_buf[0] = frame_type;
    memcpy(&frame_buf[1], &total_data_size, 2);
    const uint8_t reserved[5] = {0x05, 0x05, 0x05, 0x05, 0x05};
    memcpy(&frame_buf[3], reserved, 5);
    
    /* EOM Section */
    const uint8_t eom[EOM_SIZE] = {0x24, 0x25, 0x26};
    memcpy(p_data_section, eom, EOM_SIZE);

    /* Save total serialized frame size */
    uint16_t total_size = HEADER_SIZE + total_data_size + EOM_SIZE;
    if(frame_size){
        (*frame_size) = total_size > MAX_FRAME_SIZE? MAX_FRAME_SIZE : total_size;
    }

exit:
    return status;
}


/*
 * Extracts frame type and data size from msg.
 * TODO: If any subsection is added to header section reserved bits,
 * implement it's parsing here.
 */
int MCTP_ParseMsg(uint8_t *msg, int msg_size, MCTP_Frame *frame){
    int status = 0;
    if(msg_size < HEADER_SIZE + EOM_SIZE){
        status = -1;
        goto exit;
    }

    memset(frame, 0, sizeof(MCTP_Frame));

    frame->type = msg[0];
    memcpy(&frame->dataSize, &msg[1], 2);
    if(frame->dataSize){
        /* 
         * TODO: If any packet with data section needs to be parsed
         * by performer. Implement parsing here.
         */
    }
    
exit:
    return status;
}
