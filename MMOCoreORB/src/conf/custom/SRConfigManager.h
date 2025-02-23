#ifndef SR_CONFIG_MANAGER_H_
#define SR_CONFIG_MANAGER_H_

#include "conf/ConfigManager.h"

class SRConfigManager : public ConfigManager {
public:
    SRConfigManager();

    bool getStructurePackupEnabled();
    bool getInactiveStructurePackupEnabled();
    int getInactiveStructurePackupDays();

private:
    void loadCustomConfig();
};

#endif // SR_CONFIG_MANAGER_H_