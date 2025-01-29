#ifndef MCTP_API_H
#define MCTP_API_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "mctp.h"
#include "mctp_task.h"


/* MCTP communication functions */
int MCTP_Init(MCTP_Handle *hmctp);
int MCTP_Start(MCTP_Handle *hmctp);
void MCTP_Stop(MCTP_Handle *hmctp);
void MCTP_Notify(MCTP_Handle *hmctp, E_MCTP_Signal sig);
int MCTP_SendAll(MCTP_Handle *hmctp);
/* Channel functions */
int MCTP_EnableChannel(MCTP_Handle *hmctp, uint8_t channel_id, uint8_t *data_buf, uint16_t buf_size, E_MCTP_DataType data_type);
void MCTP_DisableChannel(MCTP_Handle *hmctp, uint8_t channel_id);
int MCTP_WriteChannelData(MCTP_Handle *hmctp, uint8_t channel_id, uint8_t *src_buf, uint16_t src_size);
void MCTP_ClearChannelData(MCTP_Handle *hmctp, uint8_t channel_id);
void MCTP_ClearChannelList(MCTP_Handle *hmctp);

#endif
