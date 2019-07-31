/*
 * common.h
 *
 * Created: 30/06/2019 21.03.52
 *  Author: pihlstro
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define foreach(idxpvar, col) \
    typeof(col[0])* idxpvar;  \
    for (idxpvar = col; idxpvar < (col + (arraylen(col))); idxpvar++)
#define arraylen(ary) (sizeof(ary) / sizeof(ary[0]))

uint16_t get_temperature(void);
unsigned int append_crc(uint8_t* data, uint32_t len);

void power_enable(void);
void power_disable(void);
uint16_t get_vbatt(void);
uint16_t get_vrpi_3v3(void);
bool vbatt_is_low(void);

#endif /* COMMON_H_ */