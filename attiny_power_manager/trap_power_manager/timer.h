/*
 * timer.h
 *
 * Created: 30/06/2019 21.24.01
 *  Author: pihlstro
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>

typedef void (*onTimerFunc)(void);

void timer_init(void);
void timer_schedule_periodical(uint16_t secs, onTimerFunc cb);
uint32_t timer_now(void);

#endif /* TIMER_H_ */