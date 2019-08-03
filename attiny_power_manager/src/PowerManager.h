#pragma once

#include <Arduino.h>
#include "IPowerState.h"
#include "PoweredState.h"
#include "UnpoweredState.h"
#include "debug.h"
#include "pmtypes.h"

class PowerManager {
   public:
    enum PowerState { ON, OFF };
    enum WakeupReason { RPI_WAKEUP_EXT_TRIGGER, RPI_WAKEUP_LOW_BATTERY };

    PowerManager() {
        _state = new UnpoweredState();
        _next_allowed_poweron = millis() + 10 * 1000;
        DBG_PRINTF("_next_allowed_poweron = %lu\n", _next_allowed_poweron);
    }

    bool is_powered(void) { return _state->is_powered(); }

    void ext_event(enum IPowerState::ext_source src) {
        _state->ext_event(*this, src);
    }

    void rpi_event(void) { _state->rpi_event(*this); }

    void timer_event(void) {
        static unsigned long next_trace_allowed = 10000;
        _temperature = get_temperature();
        _vbatt = get_vbatt();
        _vrpi = get_vrpi_3v3();
        if (next_trace_allowed < millis()) {
            DBG_PRINTF("vb %lu mV, vr %lu mV, %lu C\n", _vbatt, _vrpi,
                       _temperature - 273);
            next_trace_allowed = millis() + 30 * 1000;
        }
        _state->timer_event(*this);
    }

    setState(enum PowerState state) {
        switch (state) {
            case ON:
                if (_next_allowed_poweron < millis()) {
                    delete _state;
                    _state = new PoweredState();
                    _next_allowed_poweron = millis() + RPI_POWERON_PROHIBIT_MS;
                } else {
                    DBG_PRINTF("next allowed power on in %u ms\n",
                               _next_allowed_poweron - millis());
                }
                break;
            case OFF:
                delete _state;
                _state = new UnpoweredState();
                break;
            default:
                break;
        }
    }

    enum WakeupReason get_reason(void) {
        DBG_PRINTF("reason = %u\n", _reason);
        return _reason;
    }

    void set_reason(enum WakeupReason reason) {
        DBG_PRINTF("reason = %u\n", reason);
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