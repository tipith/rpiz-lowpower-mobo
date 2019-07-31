#pragma once

#include <Arduino.h>
#include "IPowerState.h"
#include "PoweredState.h"
#include "UnpoweredState.h"
#include "pmtypes.h"

class PowerManager {
   public:
    enum PowerState { ON, OFF };
    enum WakeupReason { RPI_WAKEUP_EXT_TRIGGER, RPI_WAKEUP_LOW_BATTERY };

    PowerManager() {
        _state = new UnpoweredState();
        _next_allowed_poweron = millis() + 10 * 1000;
    }

    void ext_event(enum IPowerState::ext_source src) {
        _state->ext_event(*this, src);
    }
    void rpi_event(void) { _state->rpi_event(*this); }
    void timer_event(void) { _state->timer_event(*this); }

    setState(enum PowerState state) {
        switch (state) {
            case ON:
                if (_next_allowed_poweron < millis()) {
                    delete _state;
                    _state = new PoweredState();
                    _next_allowed_poweron = millis() + RPI_POWERON_PROHIBIT_SEC;
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

    void setReason(enum WakeupReason reason) {}

   private:
    IPowerState* _state;
    unsigned long _next_allowed_poweron;
};