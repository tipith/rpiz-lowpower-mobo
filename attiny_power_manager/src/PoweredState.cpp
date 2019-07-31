#include "PoweredState.h"
#include "PowerManager.h"
#include "pmtypes.h"
#include "utils.h"

PoweredState::PoweredState(void) {
    power_enable();
    _next_poweroff_ms = millis() + RPI_SHUTDOWN_TIMEOUT_SEC;
}

void PoweredState::ext_event(PowerManager& pm,
                             enum IPowerState::ext_source src) {}

void PoweredState::rpi_event(PowerManager& pm) {
    _next_poweroff_ms = millis() + RPI_SHUTDOWN_DELAY_SEC;
}

void PoweredState::timer_event(PowerManager& pm) {
    if (_next_poweroff_ms < millis()) {
        pm.setState(PowerManager::OFF);
    }
}
