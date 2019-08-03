#pragma once

#include "IPowerState.h"
#include "pmtypes.h"
#include "utils.h"

class PoweredState : public IPowerState {
   public:
    PoweredState();
    void ext_event(PowerManager& pm, enum IPowerState::ext_source src);
    void rpi_event(PowerManager& pm);
    void timer_event(PowerManager& pm);
    bool is_powered(void) { return true; }

   private:
    unsigned long _next_poweroff_ms;
};