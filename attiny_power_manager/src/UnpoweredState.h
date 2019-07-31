#pragma once

#include "IPowerState.h"

class UnpoweredState : public IPowerState {
   public:
    UnpoweredState();
    void ext_event(PowerManager& pm, enum IPowerState::ext_source src);
    void rpi_event(PowerManager& pm);
    void timer_event(PowerManager& pm);
};