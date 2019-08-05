#pragma once

#include <Arduino.h>
#include "IPowerState.h"
#include "PoweredState.h"
#include "UnpoweredState.h"
#include "debug.h"
#include "pmtypes.h"

class PowerManager
{
   public:
    enum PowerState
    {
        ON,
        OFF
    };
    enum WakeupReason
    {
        RPI_WAKEUP_EXT_TRIGGER,
        RPI_WAKEUP_LOW_BATTERY
    };

    PowerManager()
    {
        _state = new UnpoweredState();
        _next_allowed_poweron = millis() + 1000;
    }

    bool is_powered(void) { return _state->is_powered(); }

    void ext(enum IPowerState::ext_source src) { _state->ext(*this, src); }

    void request_shutdown(unsigned long delay_sec) { _state->request_shutdown(*this, delay_sec); }

    void timer(void)
    {
        blink(5);
        static unsigned long next_trace_allowed = 10000;
        static unsigned long next_monitor_allowed = 10000;

        /* limit the amount of work in unpowered state to conserve energy */
        if (next_monitor_allowed < millis() || is_powered())
        {
            next_monitor_allowed = millis() + 10000;
            blink(100);
            _vbatt = get_vbatt();
            if (is_powered())
            {
                _temperature = get_temperature();
                _vrpi = get_vrpi_3v3();
                if (next_trace_allowed < millis())
                {
                    DBG_EXT_PRINTF("vb %lu mV, vr %lu mV, %lu C\n", _vbatt, _vrpi, _temperature - 273);
                    next_trace_allowed = millis() + 30 * 1000;
                }
            }
            _state->timer(*this);
        }
    }

    setState(enum PowerState state)
    {
        switch (state)
        {
            case ON:
                if (_next_allowed_poweron < millis())
                {
                    cli();
                    delete _state;
                    _state = new PoweredState();
                    sei();
                    _next_allowed_poweron = millis() + RPI_POWERON_PROHIBIT_MS;
                }
                else
                {
                    DBG_EXT_PRINTF("next on, %u ms\n", _next_allowed_poweron - millis());
                }
                break;
            case OFF:
                cli();
                delete _state;
                _state = new UnpoweredState();
                sei();
                break;
            default:
                break;
        }
    }

    enum WakeupReason get_reason(void)
    {
        DBG_EXT_PRINTF("%u\n", _reason);
        return _reason;
    }

    void set_reason(enum WakeupReason reason)
    {
        DBG_EXT_PRINTF("%u\n", reason);
        _reason = reason;
    }

    unsigned long vbatt(void) { return _vbatt; }
    unsigned long vrpi(void) { return _vrpi; }
    unsigned long temperature(void) { return _temperature; }

   private:
    IPowerState* _state;
    enum WakeupReason _reason;
    unsigned long _next_allowed_poweron;
    unsigned long _vbatt, _vrpi, _temperature;
};