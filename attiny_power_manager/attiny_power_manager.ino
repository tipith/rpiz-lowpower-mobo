#include <Wire.h>
#include <avr/sleep.h>
#include "src/PowerManager.h"
#include "src/pmtypes.h"
#include "src/utils.h"

char data[32];
unsigned int out_len = 8;
unsigned long vbatt, vrpi, temperature;
PowerManager pm;

void requestEvent() {
    digitalWrite(PIN_LED, digitalRead(PIN_LED) ? LOW : HIGH);
    Wire.write(data, out_len);
}

void receiveEvent(int howMany) {
    int reg = Wire.read();
    out_len = 0;

    switch (reg) {
        case REG_VBATT:
            data[out_len++] = vbatt & 0xff;
            data[out_len++] = (vbatt >> 8) & 0xff;
            break;
        case REG_VRPI:
            data[out_len++] = vrpi & 0xff;
            data[out_len++] = (vrpi >> 8) & 0xff;
            break;
        case REG_STARTUP_REASON:
            data[out_len++] = 2;
            data[out_len++] = 0;
            break;
        case REG_DEBUG_DATA:
            strcpy(data, "moi");
            out_len = strlen(data);
            break;
        case REG_TEMPERATURE:
            data[out_len++] = temperature & 0xff;
            data[out_len++] = (temperature >> 8) & 0xff;
            break;
    }

    out_len = append_crc(data, out_len);
}

void event_ext0(void) {
    pm.ext_event(IPowerState::EXT_SOURCE_1);
}

void event_ext1(void) {
    pm.ext_event(IPowerState::EXT_SOURCE_1);
}

void event_rpi(void) {
    pm.rpi_event();
}

void setup() {
    Wire.begin(I2C_ADDR);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    attachInterrupt(digitalPinToInterrupt(PIN_IO0), event_ext0, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_IO1), event_ext1, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_RPI_EVENT), event_rpi, FALLING);
    pinMode(PIN_PWR_EN, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_PWR_EN, LOW);
}

void loop() {
    vbatt = get_vbatt();
    vrpi = get_vrpi_3v3();
    temperature = get_temperature();
    pm.timer_event();
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_cpu();
}
