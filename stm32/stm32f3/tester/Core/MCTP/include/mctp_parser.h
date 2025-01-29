#ifndef MCTP_PARSER_H
#define MCTP_PARSER_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "mctp.h"

typedef enum{
    FRAMETYPE_NONE        = 0,
    FRAMETYPE_SYNC        = 1,
    FRAMETYPE_SYNC_RESP   = 2,
    FRAMETYPE_ACK         = 3,
    FRAMETYPE_REQUEST     = 4,
    FRAMETYPE_DATA        = 5,
    FRAMETYPE_STOP        = 6,
    FRAMETYPE_DROP        = 7,
} E_MCTP_FrameType;

typedef struct{
    E_MCTP_FrameType type;
    /* Data */
    uint16_t dataSize;
    uint8_t *dataSection;   
} MCTP_Frame;

/*
 * Parses raw <msg> to <frame> struct.
 *
 * Frames with data section such as DATA and SYNC_RESP are not
 * parsed due to not being used by performer.
 *
 * Returns 0 on success and -1 on error
 */
int MCTP_ParseMsg(uint8_t *msg, int msg_size, MCTP_Frame *frame);

/*
 * Create serialized frame of <frame_type> type and stores it on
 * <frame_buf>. The frame size is stored on <frame_size>.
 *
 * For DATA and SYNC_RESP frames, the data section is created based
 * on channel list inside <hmctp>
 *
 * Returns 0 on success and -1 on error
 */
int MCTP_Serialize(MCTP_Handle *hmctp, E_MCTP_FrameType frame_type, uint8_t *frame_buf, int frame_buf_size, uint16_t *frame_size);

#endif
