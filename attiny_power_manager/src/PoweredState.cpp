#include "PoweredState.h"
#include "PowerManager.h"
#include "debug.h"
#include "pmtypes.h"
#include "utils.h"

PoweredState::PoweredState(void)
{
    power_enable();
    _next_poweroff_ms = millis() + RPI_SHUTDOWN_TIMEOUT_MS;
}

void PoweredState::ext(PowerManager& pm, enum IPowerState::ext_source src) {}

void PoweredState::request_shutdown(PowerManager& pm, unsigned long delay_sec)
{
    unsigned long candidate = millis() + delay_sec * 1000;
    if (candidate < _next_poweroff_ms)
        _next_poweroff_ms = candidate;
}

void PoweredState::timer(PowerManager& pm)
{
    DBG_EXT_PRINTF("off in %lu\n", _next_poweroff_ms - millis());
    if (_next_poweroff_ms < millis())
    {
        pm.setState(PowerManager::OFF);
    }
}
