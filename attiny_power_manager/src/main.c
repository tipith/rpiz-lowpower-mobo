#if 0

/*
 * trap_power_manager.c
 *
 * Created: 30/06/2019 19.12.51
 * Author : pihlstro
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "adc.h"
#include "gpio.h"
#include "statemachine.h"
#include "timer.h"





enum wake_event {
	EVENT_NONE,
	EVENT_EXT1,
	EVENT_EXT2,
	EVENT_RPI,
	EVENT_TIMER
};

bool timer_occured = false;
bool ext1_occured = false;
bool ext2_occured = false;
bool rpi_occured = false;

static void _sleepy(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();
}

static void _on_ext_trigger1(void)
{
	ext1_occured = true;
	cli();
}

static void _on_ext_trigger2(void)
{
	ext2_occured = true;
	cli();
}

static void _on_rpi(void)
{
	rpi_occured = true;
}

static void _on_timer(void)
{
	timer_occured = true;
}

enum wake_event inspect_and_clear_event_source(void)
{
	if (ext1_occured)
	{
		ext1_occured = false;
		return EVENT_EXT1;
	}
	
	if (ext2_occured)
	{
		ext2_occured = false;
		return EVENT_EXT2;
	}
	
	if (rpi_occured)
	{
		rpi_occured = false;
		return EVENT_RPI;
	}

	if (timer_occured)
	{
		timer_occured = false;
		return EVENT_TIMER;
	}

	return EVENT_NONE;
}

void _sleep(uint32_t n)
{
	for(uint32_t t1 = 0; t1 < n; ++t1)
	{
		for(uint32_t t2 = 0; t2 < 80000; ++t2);
	}
}

void blink(uint32_t n)
{
	for(uint32_t t1 = 0; t1 < n; ++t1)
	{
		gpio_set(PIN_LED);
		for(uint32_t t2 = 0; t2 < 40000; ++t2);
		gpio_clear(PIN_LED);
		for(uint32_t t3 =0; t3 < 40000; ++t3);
	}
}

int main(void)
{
	gpio_init();
	gpio_subscribe(PIN_IO0, _on_ext_trigger1);
	gpio_subscribe(PIN_IO1, _on_ext_trigger2);
	gpio_subscribe(PIN_RPI_EVENT, _on_rpi);

	
	adc_init();
	
	timer_init();
	timer_schedule_periodical(60, _on_timer);
	
	statemachine_init();

	_sleep(3);
	blink(1);

    while (1) 
    {
		sei();
		//_sleepy();
		
		switch (inspect_and_clear_event_source())
		{
			case EVENT_EXT1:
				statemachine_ext(EXT_SOURCE_1);
				//blink(1);
				break;
			case EVENT_EXT2:
				statemachine_ext(EXT_SOURCE_2);
				//blink(2);
				break;
			case EVENT_RPI:
				statemachine_rpi();
				blink(3);
				break;
			case EVENT_TIMER:
				statemachine_timer();
				blink(4);
				break;
			default:
				break;
		}

		_sleep(3);
    }
}

#endif