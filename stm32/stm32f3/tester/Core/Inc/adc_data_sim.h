#ifndef ADC_DATA_SIM_H
#define ADC_DATA_SIM_H

#include "fgen.h"
#include <string.h>
#include "mctp_api.h"

void mctp_sig_callback(E_MCTP_Signal sig);
void ADCdata_initChannels(MCTP_Handle *hmctp);
void ADCdata_test_generate(void);
void ADCdata_test_send(MCTP_Handle *hmctp);

#endif
