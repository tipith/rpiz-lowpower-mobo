#include "utils.h"
#include <Arduino.h>
#include <util/crc16.h>
#include "pmtypes.h"

uint16_t get_temperature(void) {
    analogReference(INTERNAL1V1);
    delay(1);
    ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;
    ADC0.CTRLC &= ~(ADC_INITDLY_gm);
    ADC0.CTRLD = ADC_INITDLY_DLY256_gc;
    ADC0.SAMPCTRL = 31;
    ADC0.CTRLC |= ADC_SAMPCAP_bm;
    ADC0.COMMAND = ADC_STCONV_bm;
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
        ;
    uint16_t adc_reading = ADC0.RES;
    analogReference(INTERNAL2V5);

    int8_t sigrow_offset = SIGROW.TEMPSENSE1;
    uint8_t sigrow_gain = SIGROW.TEMPSENSE0;

    uint32_t temp = adc_reading - sigrow_offset;
    temp *= sigrow_gain;  // Result might overflow 16 bit variable (10bit+8bit)
    temp += 0x80;         // Add 1/2 to get correct rounding on division below
    temp >>= 8;           // Divide result to get Kelvin
    uint16_t temperature_in_K = temp;

    return temperature_in_K;
}

unsigned int append_crc(uint8_t* data, uint32_t len) {
#define CRC_LEN 2
    uint16_t crc = 0;
    for (unsigned int i = 0; i < len; ++i) {
        crc = _crc_xmodem_update(crc, data[i]);
    }

    data[len] = crc & 0xff;
    data[len + 1] = (crc >> 8) & 0xff;
    return len + CRC_LEN;
}

void power_enable(void) {
    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_PWR_EN, LOW);
}

void power_disable(void) {
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_PWR_EN, HIGH);
}

uint16_t get_vbatt(void) {
    analogReference(INTERNAL2V5);
    return map(analogRead(PIN_ADC_VBATT), 0, 1024, 0, 5250);
}

uint16_t get_vrpi_3v3(void) {
    analogReference(INTERNAL2V5);
    return map(analogRead(PIN_ADC_RPI_3V3), 0, 1024, 0, 3750);
}

bool vbatt_is_low(void) {
    return get_vbatt() < 3400;
}