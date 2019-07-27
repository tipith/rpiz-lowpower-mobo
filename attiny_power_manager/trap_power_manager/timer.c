/*
 * timer.c
 *
 * Created: 30/06/2019 21.23.24
 *  Author: pihlstro
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"

struct timer_context {
	onTimerFunc cb;
	uint16_t next_cb;
	uint16_t mins;
};

static struct timer_context timer = {0};

static inline void _wait_ctrla(void)
{
	while (RTC.STATUS & RTC_CTRLABUSY_bm);
}

static inline void _wait_cnt(void)
{
	while (RTC.STATUS & RTC_CNTBUSY_bm);
}

static inline void _wait_cmp(void)
{
	while (RTC.STATUS & RTC_CMPBUSY_bm);
}

static inline void _wait_per(void)
{
	while (RTC.STATUS & RTC_PERBUSY_bm);
}

static inline void _wait_pitctrl(void)
{
	while (RTC.PITSTATUS & RTC_CTRLBUSY_bm);
}

ISR(RTC_CNT_vect)
{
	timer.mins++;
	if (timer.cb)
	{
		timer.cb();
	}
	reti();
}

ISR(RTC_PIT_vect)
{
	reti();
}


/* public */
void timer_init(void)
{
	RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

	_wait_ctrla();
	RTC.CTRLA = RTC_RUNSTDBY_bm | RTC_PRESCALER_DIV32768_gc | RTC_RTCEN_bm;
	RTC.INTCTRL = RTC_OVF_bm;
	
	_wait_per();
	RTC.PER = 60;

	//_wait_pitctrl();
	//RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;
	//RTC.PITINTCTRL = RTC_PI_bm;
}

void timer_schedule_periodical(uint16_t secs, onTimerFunc cb)
{
	
	
}

uint32_t timer_now(void)
{
	_wait_cnt();
	uint16_t secs = (RTC.CNTH << 8) | RTC.CNTL;
	return 60 * timer.mins + secs;
}