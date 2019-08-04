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

void PoweredState::rpi(PowerManager& pm)
{
    _next_poweroff_ms = millis() + RPI_SHUTDOWN_DELAY_MS;
    DBG_EXT_PRINTF("\n");
}

void PoweredState::timer(PowerManager& pm)
{
    DBG_EXT_PRINTF("off in %lu\n", _next_poweroff_ms - millis());
    if (_next_poweroff_ms < millis())
    {
        pm.setState(PowerManager::OFF);
    }
}
