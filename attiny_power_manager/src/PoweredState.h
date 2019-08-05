#pragma once

#include "IPowerState.h"
#include "pmtypes.h"
#include "utils.h"

class PoweredState : public IPowerState
{
   public:
    PoweredState();
    void ext(PowerManager& pm, enum IPowerState::ext_source src);
    void request_shutdown(PowerManager& pm, unsigned long delay_sec);
    void timer(PowerManager& pm);
    bool is_powered(void) { return true; }

   private:
    unsigned long _next_poweroff_ms;
};