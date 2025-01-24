#ifndef MSG_H
#define MSG_H

#include "stm32f3xx_hal.h"
#include <stdio.h>
#include <string.h>

void processMsg(UART_HandleTypeDef *huart, uint8_t *msg);

#endif
