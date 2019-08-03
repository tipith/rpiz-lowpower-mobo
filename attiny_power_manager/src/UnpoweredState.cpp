#include "UnpoweredState.h"
#include "PowerManager.h"
#include "debug.h"
#include "pmtypes.h"
#include "utils.h"

UnpoweredState::UnpoweredState() {
    DBG_PRINTF("\n");
    power_disable();
}

void UnpoweredState::ext_event(PowerManager& pm,
                               enum IPowerState::ext_source src) {
    DBG_PRINTF("source = %u\n", src);
    pm.setState(PowerManager::ON);
    pm.set_reason(PowerManager::RPI_WAKEUP_EXT_TRIGGER);
}

void UnpoweredState::rpi_event(PowerManager& pm) {
    DBG_PRINTF("\n");
}

void UnpoweredState::timer_event(PowerManager& pm) {
    static bool alarmed = false;

    if (vbatt_is_low()) {
        if (!alarmed) {
            DBG_PRINTF("vbatt low\n");
            pm.setState(PowerManager::ON);
            pm.set_reason(PowerManager::RPI_WAKEUP_LOW_BATTERY);
            alarmed = true;
        } else {
            DBG_PRINTF("low vbatt alarm prevented\n");
        }
    }
}
