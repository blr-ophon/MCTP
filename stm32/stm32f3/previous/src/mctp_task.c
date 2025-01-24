/*
 * The MCTP communication task is a finite state machine whose 
 * state changes are triggered by new frames received (via
 * MCTP_ReceiveFrame) or by user notifications (via MCTP_Notify).
 * Both trigger state change by calling MCTP_updateTask.
 *
 * The finite state machine reacts to 2 types of events, received
 * frames and notifications
 */

#include "mctp_task.h"

extern MCTP_Handle *g_Hmctp;


static void MCTP_ReceiveFrame(MCTP_Handle *hmctp);
static int NotifyHandler(MCTP_Handle *hmctp);
static int FrameRecvHandler(MCTP_Handle *hmctp);

/**
 * @brief Callback for RX complete. 
 * @note Called during task to receive bytes individually
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(g_Hmctp->running){
        MCTP_ReceiveFrame(g_Hmctp);
    }
}


/**
 * @brief Checks received bytes for MCTP EOM delimiter.
 *
 * Appends all bytes received in UART port to recvBuf. Checks for end of message.
 * When a full message is received, update task.
 *
 * @param hmctp Handle for MCTP communication.
 *
 * @return None
 */
static void MCTP_ReceiveFrame(MCTP_Handle *hmctp) {
    /* Previous message was not processed or Buffer is full */
    if(hmctp->recvBufIndex >= RECV_BUFFER_SIZE -1) {
        /* Discard all in buffer */
        hmctp->recvBufIndex = 0;
        goto exit;
    }

    hmctp->recvBufIndex ++;

    /* Check for EOM after minimum size is received */
    if(hmctp->recvBufIndex >= HEADER_SIZE + EOM_SIZE){
        /* EOM = 0x24, 0x25, 0x26 ($%&) */
        uint8_t *EOMsection = &(hmctp->recvBuf[hmctp->recvBufIndex-3]);
        if(EOMsection[0] == 0x24 && EOMsection[1] == 0x25 && EOMsection[2] == 0x26){

            /* Disable IT to prevent another interrupt during parsing */
            __HAL_UART_DISABLE_IT(hmctp->huart, UART_IT_RXNE);

            /* Notify MCTP communication task*/
            MCTP_updateTask(hmctp, EVENT_FRAME_RECV);

            __HAL_UART_ENABLE_IT(hmctp->huart, UART_IT_RXNE);

            /* Clear buffer for next message */
            memset(hmctp->recvBuf, 0, RECV_BUFFER_SIZE);
            hmctp->recvBufIndex = 0;
        }
    }

exit:
    HAL_UART_Receive_IT(hmctp->huart, &hmctp->recvBuf[hmctp->recvBufIndex], 1);
}


/**
 * Update MCTP communication task finite state machine.
 * The task checks for last received frame and/or flags on <hmctp>
 * depending on it's current state, then change state accordingly.
 * Returns 0 on success and -1 on error.
 */
int MCTP_updateTask(MCTP_Handle *hmctp, E_MCTP_TaskEvent event){
    int status = 0;

    /* Notifications Events */
    if(event == EVENT_NOTIF){
        if((status = NotifyHandler(hmctp)) < 0){
            goto exit;
        }

    /* Received Frame Events*/
    }else if(event == EVENT_FRAME_RECV){
        if((status = FrameRecvHandler(hmctp)) < 0){
            goto exit;
        }
    }

exit:
    return status;
}

static int NotifyHandler(MCTP_Handle *hmctp){
    int status = 0;
    if(hmctp->userHalt && hmctp->state == STATE_TRANS){   
        /* User-triggered stop */

        /* Send STOP frame to controller */
        uint8_t stop_frame[HEADER_SIZE + EOM_SIZE];
        MCTP_Serialize(hmctp, FRAMETYPE_STOP, stop_frame, HEADER_SIZE + EOM_SIZE, NULL);
        HAL_UART_Transmit(hmctp->huart, stop_frame, HEADER_SIZE + EOM_SIZE, HAL_MAX_DELAY);

        /* Return to connected state */
        hmctp->state = STATE_CONN;
        hmctp->userHalt = false;

    }else if(hmctp->userReady){
        /* TODO: send RDY frame */
    }

exit:
    /* TODO: Chech transmit errors */
    return status;
}

int FrameRecvHandler(MCTP_Handle *hmctp){
    int status = 0;

    MCTP_Frame last_frame;
    if(MCTP_ParseMsg(hmctp->recvBuf, hmctp->recvBufIndex, &last_frame) < 0){
        status = -1;
        goto exit;
    }

    switch(hmctp->state){
        case STATE_IDLE:
            /* Idle. Waiting for SYNC packet */

            if(last_frame.type == FRAMETYPE_SYNC){
                hmctp->state = STATE_SYNC;

                /* Respond SYNC packet */
                uint8_t sync_resp_frame[SYNCRESP_FRAME_SIZE];
                if(MCTP_Serialize(hmctp, FRAMETYPE_SYNC_RESP, sync_resp_frame, SYNCRESP_FRAME_SIZE, NULL) < 0){
                    break;
                }
                HAL_UART_Transmit_IT(hmctp->huart, sync_resp_frame, SYNCRESP_FRAME_SIZE);
            }else if(last_frame.type == FRAMETYPE_DROP){
                uint8_t drop_frame[MIN_FRAME_SIZE];
                if(MCTP_Serialize(hmctp, FRAMETYPE_SYNC_RESP, drop_frame, MIN_FRAME_SIZE, NULL) < 0){
                    break;
                }
                HAL_UART_Transmit_IT(hmctp->huart, drop_frame, MIN_FRAME_SIZE);
            }
            break;

        case STATE_SYNC:
            /* Waiting for Acknowledge */

            if(last_frame.type == FRAMETYPE_ACK){
                /* Switch to connected state */
                hmctp->state = STATE_CONN;

            }else if(last_frame.type == FRAMETYPE_DROP){
                hmctp->state = STATE_IDLE

                uint8_t drop_frame[MIN_FRAME_SIZE];
                if(MCTP_Serialize(hmctp, FRAMETYPE_SYNC_RESP, drop_frame, MIN_FRAME_SIZE, NULL) < 0){
                    break;
                }
                HAL_UART_Transmit_IT(hmctp->huart, drop_frame, MIN_FRAME_SIZE);
            }

            break;

        case STATE_CONN:
            /* Connected. Waiting for Request frame*/

            if(last_frame.type == FRAMETYPE_REQUEST){
                hmctp->state = STATE_TRANS;
                /* Notify user of start request */
                hmctp->UserNotifyCallback(NOTIFY_START);

            }else if(last_frame.type == FRAMETYPE_DROP){
                hmctp->state = STATE_IDLE

                uint8_t drop_frame[MIN_FRAME_SIZE];
                if(MCTP_Serialize(hmctp, FRAMETYPE_SYNC_RESP, drop_frame, MIN_FRAME_SIZE, NULL) < 0){
                    break;
                }
                HAL_UART_Transmit_IT(hmctp->huart, drop_frame, MIN_FRAME_SIZE);
            }

            /* 
             * TODO: Once PING is implemented. Respond to PING 
             * frames here to keep connection alive.
             */
            break;

        case STATE_TRANS:
            /* Allow data until stop is called */

            if(last_frame.type == FRAMETYPE_STOP){     
                /* Controller-triggered stop */

                /* Signal stop request from controller. Wait for user halt */
                hmctp->UserNotifyCallback(NOTIFY_STOP);
            
            }else if(last_frame.type == FRAMETYPE_DROP){
                hmctp->UserNotifyCallback(NOTIFY_STOP);
                hmctp->state = STATE_IDLE

                uint8_t drop_frame[MIN_FRAME_SIZE];
                if(MCTP_Serialize(hmctp, FRAMETYPE_SYNC_RESP, drop_frame, MIN_FRAME_SIZE, NULL) < 0){
                    break;
                }
                HAL_UART_Transmit_IT(hmctp->huart, drop_frame, MIN_FRAME_SIZE);
            }
            break;
    }
exit:
    return status;
}
