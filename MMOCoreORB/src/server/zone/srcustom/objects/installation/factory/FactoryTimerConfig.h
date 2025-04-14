#ifndef FACTORY_TIMER_CONFIG_H_
#define FACTORY_TIMER_CONFIG_H_

#include "server/zone/srcustom/managers/configuration/SRConfigManager.h"

/**
 * @brief Configuration class for factory timer settings
 * 
 * This class provides configuration for factory timer multipliers
 * using the SRConfigManager for runtime configuration.
 */
class FactoryTimerConfig {
public:
    static int getTimerMultiplier() {
        return SRConfigManager::instance()->getFactoryTimerMultiplier();
    }
};

#endif // FACTORY_TIMER_CONFIG_H_