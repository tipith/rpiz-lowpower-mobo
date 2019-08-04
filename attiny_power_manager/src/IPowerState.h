#pragma once

class PowerManager;

class IPowerState
{
   public:
    enum ext_source
    {
        EXT_SOURCE_1,
        EXT_SOURCE_2
    };

    virtual void ext(PowerManager& pm, enum ext_source src) = 0;
    virtual void rpi(PowerManager& pm) = 0;
    virtual void timer(PowerManager& pm) = 0;
    virtual bool is_powered(void) = 0;
};