#include <Wire.h>
#include <avr/sleep.h>
#include "src/DebugLogger.h"
#include "src/PowerManager.h"
#include "src/debug.h"
#include "src/pmtypes.h"
#include "src/utils.h"

#define I2C_BUFFER_SIZE 32
#define I2C_PAYLOAD_SIZE (I2C_BUFFER_SIZE - 2)  // 2 bytes crc

static uint8_t out_data[I2C_BUFFER_SIZE];
static unsigned int out_len = 8;
static PowerManager* pm;
static DebugLogger* dbg;

void requestEvent() {
    digitalWrite(PIN_LED, digitalRead(PIN_LED) ? LOW : HIGH);
    Wire.write(out_data, out_len);
}

void receiveEvent(int howMany) {
    int reg = Wire.read();
    out_len = 0;

    switch (reg) {
        case REG_VBATT:
            out_len += pack_u16(out_data, pm->vbatt());
            break;
        case REG_VRPI:
            out_len += pack_u16(out_data, pm->vrpi());
            break;
        case REG_STARTUP_REASON:
            out_len += pack_u16(out_data, pm->get_reason());
            break;
        case REG_TEMPERATURE:
            out_len += pack_u16(out_data, pm->temperature());
            break;
        case REG_DEBUG_DATA_READ: {
            memset(out_data, 0, sizeof(out_data));
            dbg->read(out_data, I2C_PAYLOAD_SIZE);
            out_len = I2C_PAYLOAD_SIZE;
            break;
        }
    }

    out_len = append_crc(out_data, out_len);
}

void event_ext0(void) {
    pm->ext_event(IPowerState::EXT_SOURCE_1);
}

void event_ext1(void) {
    pm->ext_event(IPowerState::EXT_SOURCE_2);
}

void event_rpi(void) {
    pm->rpi_event();
}

void setup() {
    dbg = new DebugLogger(256);
    debug_set_logger(dbg);
    pm = new PowerManager();

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
    pm->timer_event();
    delay(2000);

    if (pm->is_powered() == false) {
        sleep_enable();
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_cpu();
    }
}
