#pragma once

class PowerManager;

class IPowerState {
   public:
    enum ext_source { EXT_SOURCE_1, EXT_SOURCE_2 };

    virtual void ext_event(PowerManager& pm, enum ext_source src) = 0;
    virtual void rpi_event(PowerManager& pm) = 0;
    virtual void timer_event(PowerManager& pm) = 0;
};