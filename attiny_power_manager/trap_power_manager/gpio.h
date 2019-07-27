/*
 * io.h
 *
 * Created: 30/06/2019 21.23.51
 *  Author: pihlstro
 */ 


#ifndef IO_H_
#define IO_H_

enum iopin {
	PIN_IO0,
	PIN_IO1,
	PIN_IO2,
	PIN_IO3,
	PIN_RPI_EVENT,
	PIN_PWR_EN,
	PIN_LED,
	PIN_SCL,
	PIN_SDA,
	PIN_NUM
};

typedef void (*onPinTransitionFunc)(void);


void gpio_init(void);
void gpio_subscribe(enum iopin pin, onPinTransitionFunc cb);

void gpio_set(enum iopin pin);
void gpio_clear(enum iopin pin);


#endif /* IO_H_ */