/*
 * adc.c
 *
 * Created: 30/06/2019 21.02.56
 *  Author: pihlstro
 */ 
#include <stddef.h>
#include <avr/io.h>

#include "utils.h"
#include "adc.h"

#define ADC_MAXVAL (1 << 10)

struct adc_conf {
	enum adc_ch ch;
	uint8_t muxpos;
	uint16_t fullscale_mv;
};

const struct adc_conf adc_confs[] = {
	{ADC_CH_VBATT, ADC_MUXPOS_AIN1_gc, 5250},
	{ADC_CH_RPI_3V3, ADC_MUXPOS_AIN4_gc, 3750},
	{ADC_CH_TEMP, ADC_MUXPOS_TEMPSENSE_gc, 1100},

};

static const struct adc_conf* adc_find_conf(enum adc_ch ch)
{
	foreach (p2, adc_confs)
	{
		if (p2->ch == ch)
		return p2;
	}
	return NULL;
}

void adc_init(void)
{
	ADC0.CTRLA = ADC_RESSEL_10BIT_gc | ADC_ENABLE_bm;
	
	/* sample averaging */
	ADC0.CTRLB = ADC_SAMPNUM_ACC16_gc;
	
	/* reduced sample cap, internal ref  */
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL_INTREF_gc | ADC_PRESC_DIV16_gc;
	
	/* input is quite high-impedance. sample a long time */
	uint8_t sampledly = 0;
	ADC0.CTRLD = ADC_INITDLY_DLY64_gc | sampledly;
	
	/* input is quite high-impedance. sample a long time */
	ADC0.SAMPCTRL = 15;
}

bool adc_sample(enum adc_ch ch, uint16_t* outval)
{
	bool rv = false;
	const struct adc_conf* conf = adc_find_conf(ch);
	
	if (conf)
	{
		ADC0.MUXPOS = conf->muxpos;
		ADC0.COMMAND = ADC_STCONV_bm;
		
		while (ADC0.COMMAND & ADC_STCONV_bm);
		
		*outval = (ADC0.RESH << 8) | ADC0.RESL;
		*outval = (*outval * conf->fullscale_mv) / ADC_MAXVAL;
		rv = true;
	}
	
	return rv;
}