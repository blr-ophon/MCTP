#include "adc_data.h"

static float wav0_samples[100] = {0};
static float wav1_samples[100] = {0};
static float wav2_samples[100] = {0};
static float wav3_samples[100] = {0};
static float wav4_samples[100] = {0};
static float wav5_samples[100] = {0};

float ch0_buf[30] = {0};
float ch1_buf[30] = {0};
float ch2_buf[30] = {0};
float ch3_buf[30] = {0};
float ch4_buf[30] = {0};
float ch5_buf[30] = {0};
char ch6_buf[30] = {0};
char ch7_buf[30] = {0};

extern UART_HandleTypeDef huart2;

char d1_str[20] = {0};
char d2_str[20] = {0};

bool sending = false;

/*
 * NOTE: This function blocks MCTP communication task
 */
void mctp_user_callback(E_MCTP_Notification notif){
    switch(notif){
        case NOTIFY_STOP:
            sending = false;
            break;
        case NOTIFY_START:
            sending = true;
            break;
        default:
            break;
    }
}

void ADCdata_initChannels(MCTP_Handle *hmctp){
    /* Add Channels */
    MCTP_EnableChannel(hmctp, 0, (uint8_t*)ch0_buf, 30*sizeof(float), DATATYPE_FLOAT32);
    MCTP_EnableChannel(hmctp, 1, (uint8_t*)ch1_buf, 30*sizeof(float), DATATYPE_FLOAT32);
    MCTP_EnableChannel(hmctp, 2, (uint8_t*)ch2_buf, 30*sizeof(float), DATATYPE_FLOAT32);
    MCTP_EnableChannel(hmctp, 3, (uint8_t*)ch3_buf, 30*sizeof(float), DATATYPE_FLOAT32);
    MCTP_EnableChannel(hmctp, 4, (uint8_t*)ch4_buf, 30*sizeof(float), DATATYPE_FLOAT32);
    MCTP_EnableChannel(hmctp, 5, (uint8_t*)ch5_buf, 30*sizeof(float), DATATYPE_FLOAT32);
    MCTP_EnableChannel(hmctp, 6, (uint8_t*)ch6_buf, 30*sizeof(char), DATATYPE_CHAR);
    MCTP_EnableChannel(hmctp, 7, (uint8_t*)ch7_buf, 30*sizeof(char), DATATYPE_CHAR);
}

void ADCdata_test_generate(void) {
    Wave wav0 = {WAV_SQUARE, 4, 5};
    Wave wav1 = {WAV_TRIG, 4, 5};
    Wave wav2 = {WAV_SINE, 4, 5};
    Wave wav3 = {WAV_SINE, 8, 5};
    Wave wav4 = {WAV_SINE, 16, 5};
    Wave wav5 = {WAV_SINE, 32, 5};

    FGen_simple(wav0_samples, 60, wav0, 120);
    FGen_simple(wav1_samples, 60, wav1, 120);
    FGen_simple(wav2_samples, 60, wav2, 120);
    FGen_simple(wav3_samples, 60, wav3, 120);
    FGen_simple(wav4_samples, 60, wav4, 120);
    FGen_simple(wav5_samples, 60, wav5, 120);
}

void ADCdata_test_send(MCTP_Handle *hmctp){
    // uint8_t test_data[7] = {1,2,3};
    // float test_data2[7] = {1.1, 2.2, 3.3};
    // MCTP_dataAppend(hmctp, 4, (uint8_t*) test_data2, 3*sizeof(float));
    
    //for(int i = 0; sending; i++) {
    //    if(i == 100) {
    //        i = 0;
    //    }
    int frames_counter = 0;
    while(sending){
        /* Append Data to channel */
        MCTP_ClearChannelData(hmctp, 6);
        MCTP_ClearChannelData(hmctp, 7);
        MCTP_WriteChannelData(hmctp, 0, (uint8_t*)wav0_samples, 30*sizeof(float));
        MCTP_WriteChannelData(hmctp, 1, (uint8_t*)wav1_samples, 30*sizeof(float));
        MCTP_WriteChannelData(hmctp, 2, (uint8_t*)wav2_samples, 30*sizeof(float));
        MCTP_WriteChannelData(hmctp, 3, (uint8_t*)wav3_samples, 30*sizeof(float));
        MCTP_WriteChannelData(hmctp, 4, (uint8_t*)wav4_samples, 30*sizeof(float));
        MCTP_WriteChannelData(hmctp, 5, (uint8_t*)wav5_samples, 30*sizeof(float));
        if(frames_counter != 0 && frames_counter % 10 == 0){
            char *text_msg = "10 frames sent";
            MCTP_WriteChannelData(hmctp, 6, (uint8_t*)text_msg, strlen(text_msg));
        }
        else if(frames_counter != 0 && frames_counter % 5 == 0){
            char *text_msg = "5 frames sent";
            MCTP_WriteChannelData(hmctp, 7, (uint8_t*)text_msg, strlen(text_msg));
        }
        frames_counter++;


        /* Simulate data acquisition delay */
        HAL_Delay(90);
        /* send */
        MCTP_SendAll(hmctp);

        /* STOP request during delay */
        if(!sending){
            MCTP_Notify(hmctp, NOTIFY_HALT);
        }
    }
}

void oldADCdata_test_send(MCTP_Handle *hmctp){
    /*
     * TODO:
     * Final code will work this way:
     *
     * Whenever ADC fills the first half of the buffers, the halfCplt callback
     * will trigger UART to send half the buffer (the first half) via DMA.
     * Whenever ADC fills the second half of the buffers, the Cplt callback
     * will trigger UART to send half the buffer (the second half) via DMA.
     */
    for(int i = 0; sending; i++) {
        if(i == 100) {
            i = 0;
        }
        char msg[50]; // Adjust size as needed
        snprintf(d1_str, 19, "%f", wav1_samples[i]);
        snprintf(d2_str, 19, "%f", wav2_samples[i]);
        snprintf(msg, sizeof(msg), "#D#%s#%s#%d#$", d1_str, d2_str, strlen(d1_str) + strlen(d2_str));

        //printf("#D#%s#%s#%d#$", d1_str, d2_str, strlen(d1_str) + strlen(d2_str) );
        HAL_Delay(100); /* Simulate time to acquire data */
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    MCTP_Notify(hmctp, NOTIFY_HALT);

    while(!sending){

    }
}
