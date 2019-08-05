#include <Wire.h>
#include <avr/sleep.h>
#include "src/DebugLogger.h"
#include "src/PowerManager.h"
#include "src/debug.h"
#include "src/pmtypes.h"
#include "src/utils.h"

#define I2C_BUFFER_SIZE 32
#define I2C_PAYLOAD_SIZE (I2C_BUFFER_SIZE - 2)  // 2 bytes crc

extern volatile uint32_t timer_millis;

static uint8_t out_data[I2C_BUFFER_SIZE];
static unsigned int out_len = 8;
static PowerManager* pm;
static DebugLogger* dbg;

static void requestEvent()
{
    if (out_len)
        Wire.write(out_data, out_len);
}

static void receiveEvent(int howMany)
{
    int reg = Wire.read();
    out_len = 0;

    switch (reg)
    {
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
        case REG_DEBUG_DATA_READ:
            memset(out_data, 0, sizeof(out_data));
            dbg->read(out_data, I2C_PAYLOAD_SIZE);
            out_len = I2C_PAYLOAD_SIZE;
            break;
        case REG_REQUEST_SHUTDOWN:
            if (Wire.available() == 4)
            {
                uint16_t secs = Wire.read() | (Wire.read() << 8);
                uint16_t crc = Wire.read() | (Wire.read() << 8);
                if (calculate_crc16((uint8_t*)&secs, 2) == crc)
                    pm->request_shutdown(secs);
            }
            break;
    }

    if (out_len)
        out_len = append_crc16(out_data, out_len);
}

static void event_ext0(void)
{
    pm->ext(IPowerState::EXT_SOURCE_1);
}

static void event_ext1(void)
{
    pm->ext(IPowerState::EXT_SOURCE_2);
}

static void sleep_if_needed(void)
{
    cli();
    if (pm->is_powered() == false)
    {
        DBG_PRINTF("s");
        sei();
        sleep_mode();
        cli();
        timer_millis += 1000;
        sei();
        DBG_PRINTF("w");
    }
    else
    {
        sei();
        delay(1000);
    }
}

void RTC_init(void)
{
    while (RTC.STATUS > 0)
        ;
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;                    /* 32.768kHz Internal Crystal */
    RTC.PITINTCTRL = RTC_PI_bm;                           /* PIT Interrupt: enabled */
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm; /* 1 Hz */
}

ISR(RTC_PIT_vect)
{
    RTC.PITINTFLAGS = RTC_PI_bm; /* Clear interrupt flag */
}

void setup()
{
    pinMode(PIN_PWR_EN, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    // indication to rpi if attiny is sleeping, should preent i2c polling
    pinMode(PIN_RPI_EVENT, OUTPUT);

    dbg = new DebugLogger(256);
    debug_set_logger(dbg);
    pm = new PowerManager();

    // TWI will be enabled on power on to reduce current consumption
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    attachInterrupt(digitalPinToInterrupt(PIN_IO0), event_ext0, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_IO1), event_ext1, FALLING);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    RTC_init();
}

void loop()
{
    pm->timer();
    sleep_if_needed();
}
