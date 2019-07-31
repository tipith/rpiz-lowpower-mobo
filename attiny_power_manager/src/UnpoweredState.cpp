#include "UnpoweredState.h"
#include "PowerManager.h"
#include "pmtypes.h"
#include "utils.h"

UnpoweredState::UnpoweredState() {
    power_disable();
}

void UnpoweredState::ext_event(PowerManager& pm,
                               enum IPowerState::ext_source src) {
    pm.setState(PowerManager::ON);
    pm.setReason(PowerManager::RPI_WAKEUP_LOW_BATTERY);
}

void UnpoweredState::rpi_event(PowerManager& pm) {}

void UnpoweredState::timer_event(PowerManager& pm) {
    static bool alarmed = false;

    if (!alarmed && vbatt_is_low()) {
        pm.setState(PowerManager::ON);
        pm.setReason(PowerManager::RPI_WAKEUP_LOW_BATTERY);
        alarmed = true;
    }
}
