#include "UnpoweredState.h"
#include "PowerManager.h"
#include "debug.h"
#include "pmtypes.h"
#include "utils.h"

UnpoweredState::UnpoweredState()
{
    DBG_EXT_PRINTF("\n");
    power_disable();
}

void UnpoweredState::ext(PowerManager& pm, enum IPowerState::ext_source src)
{
    DBG_EXT_PRINTF("%u\n", src);
    pm.setState(PowerManager::ON);
    pm.set_reason(PowerManager::RPI_WAKEUP_EXT_TRIGGER);
}

void UnpoweredState::rpi(PowerManager& pm) {}

void UnpoweredState::timer(PowerManager& pm)
{
    static bool alarmed = false;

    if (vbatt_is_low())
    {
        if (!alarmed)
        {
            DBG_EXT_PRINTF("vbatt low\n");
            pm.setState(PowerManager::ON);
            pm.set_reason(PowerManager::RPI_WAKEUP_LOW_BATTERY);
            alarmed = true;
        }
        else
        {
            DBG_EXT_PRINTF("low vbatt alarm prevented\n");
        }
    }
}
