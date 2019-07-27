/*
 * power.c
 *
 * Created: 30/06/2019 21.46.23
 *  Author: pihlstro
 */ 
#include "adc.h"
#include "gpio.h"
#include "power.h"

void power_enable(void)
{
	gpio_set(PIN_LED);
	gpio_clear(PIN_PWR_EN);
}

void power_disable(void)
{
	gpio_clear(PIN_LED);
	gpio_set(PIN_PWR_EN);
}

bool power_vbatt(uint16_t* vout_mv)
{
	return vout_mv && adc_sample(ADC_CH_VBATT, vout_mv);
}

bool power_rpi_3v3(uint16_t* vout_mv)
{
	return vout_mv && adc_sample(ADC_CH_RPI_3V3, vout_mv);
}

bool power_vbatt_is_low(void)
{
	uint16_t mv;
	return power_vbatt(&mv) && mv < 3400;
}