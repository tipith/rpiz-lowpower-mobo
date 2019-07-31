#pragma once

#define I2C_ADDR 8

#define RPI_SHUTDOWN_TIMEOUT_SEC (5 * 60)
#define RPI_SHUTDOWN_DELAY_SEC (30)
#define RPI_POWERON_PROHIBIT_SEC (3 * 60 * 60)

enum iopin {
    PIN_IO0 = 9,
    PIN_IO1 = 2,
    PIN_IO2 = 10,
    PIN_IO3 = 1,
    PIN_RPI_EVENT = 5,
    PIN_PWR_EN = 3,
    PIN_LED = 4,
    PIN_SCL = 7,
    PIN_SDA = 6,
    PIN_ADC_VBATT = 8,
    PIN_ADC_RPI_3V3 = 0,
};

enum i2c_reg {
    REG_VBATT = 1,
    REG_VRPI,
    REG_STARTUP_REASON,
    REG_DEBUG_DATA,
    REG_TEMPERATURE,
};