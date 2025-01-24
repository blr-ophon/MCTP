#include "msg.h"

#define RECV_BUFFER_SIZE 100

uint8_t recvBuf[RECV_BUFFER_SIZE] = {0};
uint8_t recvBufIndex = 0;
extern int data_streams_n;
extern int ADC_send;

/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // Previous message was not processed or Buffer is full
    if(recvBufIndex >= RECV_BUFFER_SIZE -1) {
        //Discard all in buffer
        recvBufIndex = 0;
        goto out;
    }

    //char *msg = "RECEIVED\r\n";
    //HAL_UART_Transmit_IT(huart, (uint8_t*)msg, strlen(msg));
    //printf("RECEIVED (i:%d)\r\n", recvBufIndex);

    uint8_t byteReceived = recvBuf[recvBufIndex++];

    if((char) byteReceived == '$') {
        __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
        processMsg(huart, recvBuf);
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
        recvBufIndex = 0;
    }
out:
    HAL_UART_Receive_IT(huart, &recvBuf[recvBufIndex], 1);
}
*/

void processMsg(UART_HandleTypeDef *huart, uint8_t *msg) {
    // TODO
    char *msg_str = (char*) msg;
    char *pMsgStart = strchr(msg_str, '#');

    if(pMsgStart == NULL) {
        //No '#' found. Discard message
        recvBufIndex = 0;
        return;
    }

    char opt = pMsgStart[1];
    switch(opt) {
    case '?':
        printf("#!#%d#$", data_streams_n);
        break;
    case 'A':
        // Send analog data
        ADC_send = SET;
        printf("Send analog data\r\n");
        break;
    case 'S':
        // Stop sending
        
        //Wait for current msg on buffer to be sent, then send STOP confirmation
        while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) {}
        printf("#S#$");

        ADC_send = RESET;
        break;
    default:
        printf("Unrecognized\r\n");
    }
}
