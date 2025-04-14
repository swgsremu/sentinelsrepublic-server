#include "SRConfigManager.h"

SRConfigManager::SRConfigManager() {
    loadConfigData();    // Load base config
    loadCustomConfig(); // Load custom config
}

bool SRConfigManager::getStructurePackupEnabled() {
    return getBool("SRConfig.structurePackupEnabled", false);
}

bool SRConfigManager::getInactiveStructurePackupEnabled() {
    return getBool("SRConfig.inactiveStructurePackupEnabled", false);
}

int SRConfigManager::getInactiveStructurePackupDays() {
    return getInt("SRConfig.inactiveStructurePackupDays", 365);
}

int SRConfigManager::getFactoryTimerMultiplier() {
    return getInt("SRConfig.factoryTimerMultiplier", 8);  // Default to 8 like the original
}

int SRConfigManager::getCraftingTimerInterval() {
    return getInt("SRConfig.craftingTimerInterval", 5);  // Default to 5 seconds
}

int SRConfigManager::getCraftingTimerIncrement() {
    return getInt("SRConfig.craftingTimerIncrement", 5000);  // Default to 5000 milliseconds
}

void SRConfigManager::loadCustomConfig() {
    const auto lua = new Lua();
    lua->init();

    if (!lua->runFile("conf/custom/config.lua")) {
        info() << "Failed to load conf/custom/config.lua";
        delete lua;
        return;
    }

    // Append custom settings from SRConfig table
    LuaObject srConfig = lua->getGlobalObject("SRConfig");
    if (srConfig.isValidTable()) {
        const String packupEnabled = srConfig.getStringField("structurePackupEnabled");
        setBool("SRConfig.structurePackupEnabled", packupEnabled == "true" ? true : false);

        const String inactivePackupEnabled = srConfig.getStringField("inactiveStructurePackupEnabled");
        setBool("SRConfig.inactiveStructurePackupEnabled", inactivePackupEnabled == "true" ? true : false);

        setInt("SRConfig.inactiveStructurePackupDays", static_cast<int>(srConfig.getIntField("inactiveStructurePackupDays")));
        setInt("SRConfig.factoryTimerMultiplier", static_cast<int>(srConfig.getIntField("factoryTimerMultiplier")));
        setInt("SRConfig.craftingTimerInterval", static_cast<int>(srConfig.getIntField("craftingTimerInterval")));
        setInt("SRConfig.craftingTimerIncrement", static_cast<int>(srConfig.getIntField("craftingTimerIncrement")));
    } else {
        error() << "SRConfig table not found in conf/custom/config.lua";
    }
    srConfig.pop();

    delete lua;
}