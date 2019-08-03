#include "PoweredState.h"
#include "PowerManager.h"
#include "debug.h"
#include "pmtypes.h"
#include "utils.h"

PoweredState::PoweredState(void) {
    power_enable();
    _next_poweroff_ms = millis() + RPI_SHUTDOWN_TIMEOUT_MS;
    DBG_PRINTF("power off in %lu\n", _next_poweroff_ms - millis());
}

void PoweredState::ext_event(PowerManager& pm,
                             enum IPowerState::ext_source src) {
    DBG_PRINTF("source = %u\n", src);
}

void PoweredState::rpi_event(PowerManager& pm) {
    _next_poweroff_ms = millis() + RPI_SHUTDOWN_DELAY_MS;
    DBG_PRINTF("next power off at %lu\n", _next_poweroff_ms);
}

void PoweredState::timer_event(PowerManager& pm) {
    DBG_PRINTF("PoweredState - power off in %lu\n",
               _next_poweroff_ms - millis());
    if (_next_poweroff_ms < millis()) {
        pm.setState(PowerManager::OFF);
    }
}
