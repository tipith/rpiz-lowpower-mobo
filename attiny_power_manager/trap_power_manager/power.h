/*
 * power.h
 *
 * Created: 30/06/2019 21.48.50
 *  Author: pihlstro
 */ 


#ifndef POWER_H_
#define POWER_H_

#include <avr/io.h>

void power_enable(void);
void power_disable(void);

bool power_vbatt(uint16_t* vout_mv);
bool power_vbatt_is_low(void);

bool power_rpi_3v3(uint16_t* vout_mv);

#endif /* POWER_H_ */