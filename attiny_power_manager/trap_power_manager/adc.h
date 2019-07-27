/*
 * adc.h
 *
 * Created: 30/06/2019 21.05.05
 *  Author: pihlstro
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdbool.h>

enum adc_ch {
	ADC_CH_VBATT,
	ADC_CH_RPI_3V3,
	ADC_CH_TEMP
};

void adc_init(void);
bool adc_sample(enum adc_ch ch, uint16_t* outval);

#endif /* ADC_H_ */