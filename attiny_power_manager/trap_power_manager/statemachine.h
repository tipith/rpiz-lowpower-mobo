/*
 * statemachine.h
 *
 * Created: 30/06/2019 21.25.05
 *  Author: pihlstro
 */ 


#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

enum ext_source {
	EXT_SOURCE_1,
	EXT_SOURCE_2
};

void statemachine_init(void);
void statemachine_ext_event(enum ext_source src);
void statemachine_rpi_event(void);
void statemachine_timer_event(void);

#endif /* STATEMACHINE_H_ */