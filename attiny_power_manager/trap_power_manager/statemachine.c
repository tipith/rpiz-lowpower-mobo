/*
 * statemachine.c
 *
 * Created: 30/06/2019 21.24.20
 *  Author: pihlstro
 */ 
#include <stddef.h>

#include "statemachine.h"
#include "adc.h"
#include "timer.h"
#include "power.h"

#define RPI_SHUTDOWN_TIMEOUT_SEC  (5*60)
#define RPI_SHUTDOWN_DELAY_SEC    (30)
#define RPI_POWERON_PROHIBIT_SEC  (3*60*60)


enum rpi_wakeup_reason {
	RPI_WAKEUP_EXT_TRIGGER,
	RPI_WAKEUP_LOW_BATTERY	
};

enum state {
	STATE_POWER_ON,
	STATE_POWER_OFF
};

typedef void(*timerFunc)(void);
typedef void(*extEventFunc)(enum ext_source src);
typedef void(*rpiEventFunc)(void);

struct state_context {
	timerFunc timer;
	extEventFunc event_ext;
	rpiEventFunc event_rpi;
	uint32_t transition_timeout;
	uint32_t next_allowed_poweron;
};

struct state_context state;

/*** state chart *************************************************************
 *
 *                                 o
 *             -- rpi ev ---       |
 *           /               \     |
 *  POWER ON                  -> POWER OFF
 *   ^   ^   \               /     /  /
 *    \   \    -- timeout --      /  /
 *     \   \                     /  /
 *      \    ---- ext trigger ---  /
 *       \                        /
 *         --- low batt (once) --- 
 *
 */


/*** internal ***************************************************************/
static void _power_on_rpi_event(void);
static void _power_on_timer_event(void);
static void _power_off_timer_event(void);
static void _power_off_ext_event(enum ext_source src);

static void _power_on_transition_to(void)
{
	state.event_rpi = _power_on_rpi_event;
	state.event_ext = NULL;
	state.timer = _power_on_timer_event;
	state.transition_timeout = timer_now() + RPI_SHUTDOWN_TIMEOUT_SEC;
	state.next_allowed_poweron = timer_now() + RPI_POWERON_PROHIBIT_SEC;
}

static void _power_off_transition_to(void)
{
	state.event_rpi = NULL;
	state.event_ext = _power_off_ext_event;
	state.timer = _power_off_timer_event;
	state.transition_timeout = 0;
}

static void _poweron(enum rpi_wakeup_reason reason)
{
	power_enable();
	_power_on_transition_to();
}

static void _poweroff(void)
{
	power_disable();
	_power_off_transition_to();
}

static void _power_on_rpi_event(void)
{
	state.transition_timeout = timer_now() + RPI_SHUTDOWN_DELAY_SEC;
}

static void _power_on_timer_event(void)
{
	if (state.transition_timeout < timer_now())
	{
		_poweroff();
	}
}

static void _power_off_timer_event(void)
{
	static bool alarmed = false;
	
	if (!alarmed && power_vbatt_is_low())
	{
		_poweron(RPI_WAKEUP_LOW_BATTERY);
		alarmed = true;
	}
}

static void _power_off_ext_event(enum ext_source src)
{
	if (state.next_allowed_poweron < timer_now())
	{
		_poweron(RPI_WAKEUP_EXT_TRIGGER);
	}
}

/*** public *****************************************************************/
void statemachine_init(void)
{
	_power_off_transition_to();
	state.next_allowed_poweron = 0;
}

void statemachine_ext_event(enum ext_source src)
{
	if (state.event_ext)
	{
		state.event_ext(src);
	}
}

void statemachine_rpi_event(void)
{
	if (state.event_rpi)
	{
		state.event_rpi();
	}
}

void statemachine_timer_event(void)
{
	if (state.timer)
	{
		state.timer();
	}
}
