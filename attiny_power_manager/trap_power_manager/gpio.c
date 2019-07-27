/*
 * io.c
 *
 * Created: 30/06/2019 21.23.41
 *  Author: pihlstro
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "gpio.h"

struct gpio_context {
	onPinTransitionFunc cb;
};

static struct gpio_context gpio[PIN_NUM];

ISR(PORTA_PORT_vect)
{
	if (PORTA.INTFLAGS & PORT_INT2_bm)
	{
		PORTA.INTFLAGS = PORT_INT2_bm;
		if (gpio[PIN_IO0].cb)
		{
			gpio[PIN_IO0].cb();
		}
	}
	
	if (PORTA.INTFLAGS & PORT_INT6_bm)
	{
		PORTA.INTFLAGS = PORT_INT6_bm;
		if (gpio[PIN_IO1].cb)
		{
			gpio[PIN_IO1].cb();
		}
	}
	
	reti();
}

ISR(PORTB_PORT_vect)
{
	if (PORTA.INTFLAGS & PORT_INT2_bm)
	{
		PORTA.INTFLAGS = PORT_INT2_bm;
		if (gpio[PIN_RPI_EVENT].cb)
		{
			gpio[PIN_RPI_EVENT].cb();
		}
	}
	
	reti();
}

void gpio_init(void)
{
	/* PA0 - UPDI
	 * PA1 - VBATT    - analog in
	 * PA2 - IO0_WAKE - digital in, async isr 
	 * PA3 - IO2      - digital in/out
	 * PA4 - RPI_3V3  - analog in
	 * PA5 - IO3      - digital in/out
	 * PA6 - IO1_WAKE - digital in, async isr
	 * PA7 - PWR_EN   - digital out */
	PORTA.DIRSET = 0b10101000;
	PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN2CTRL = PORT_ISC_FALLING_gc;
	PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN6CTRL = PORT_ISC_FALLING_gc;
	
	/* PB0 - RPI_SCL   - i2c slave
	 * PB1 - RPI_SCA   - i2c slave
	 * PB2 - RPI_EVENT - digital in, async isr
	 * PB3 - LED       - digital out */
	PORTB.DIRSET = 0b1011;
	PORTB.PIN2CTRL = PORT_ISC_FALLING_gc;
}

void gpio_subscribe(enum iopin pin, onPinTransitionFunc cb)
{
	gpio[pin].cb = cb;
}

void gpio_set(enum iopin pin)
{
	switch (pin)
	{
	case PIN_IO2:
		PORTA.OUTSET = (1 << 3);
		break;
	case PIN_IO3:
		PORTA.OUTSET = (1 << 5);
		break;
	case PIN_PWR_EN:
		PORTA.OUTSET = (1 << 7);
		break;
	case PIN_LED:
		PORTB.OUTSET = (1 << 3);
		break;
	case PIN_SCL:
		PORTB.OUTSET = (1 << 0);
		break;
	case PIN_SDA:
		PORTB.OUTSET = (1 << 1);
		break;
	default:
		break;
	}
}

void gpio_clear(enum iopin pin)
{
	switch (pin)
	{
	case PIN_IO2:
		PORTA.OUTCLR = (1 << 3);
		break;
	case PIN_IO3:
		PORTA.OUTCLR = (1 << 5);
		break;
	case PIN_PWR_EN:
		PORTA.OUTCLR = (1 << 7);
		break;
	case PIN_LED:
		PORTB.OUTCLR = (1 << 3);
		break;
	case PIN_SCL:
		PORTB.OUTCLR = (1 << 0);
		break;
	case PIN_SDA:
		PORTB.OUTCLR = (1 << 1);
		break;
	default:
		break;
	}
}