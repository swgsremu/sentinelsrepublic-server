#ifndef SR_CONFIG_MANAGER_H_
#define SR_CONFIG_MANAGER_H_

#include "conf/ConfigManager.h"

class SRConfigManager final : public Singleton<SRConfigManager>, public ConfigManager {
public:
    SRConfigManager();
    bool getStructurePackupEnabled();
    bool getInactiveStructurePackupEnabled();
    int getInactiveStructurePackupDays();
    int getFactoryTimerMultiplier();
    int getCraftingTimerInterval();  
    int getCraftingTimerIncrement(); 

    using Singleton<SRConfigManager>::instance;
private:
    void loadCustomConfig();
};

#endif // SR_CONFIG_MANAGER_H_