#pragma once

#include "IPowerState.h"

class UnpoweredState : public IPowerState
{
   public:
    UnpoweredState();
    void ext(PowerManager& pm, enum IPowerState::ext_source src);
    void rpi(PowerManager& pm);
    void timer(PowerManager& pm);
    bool is_powered(void) { return false; }
};