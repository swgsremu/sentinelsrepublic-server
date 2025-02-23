#include "SRConfigManager.h"

SRConfigManager::SRConfigManager() : ConfigManager() {
    loadConfigData(); // Load base config
    loadCustomConfig(); // Load custom config
}

bool SRConfigManager::getStructurePackupEnabled() {
    return getBool("Core3.structurePackupEnabled", false);
}

bool SRConfigManager::getInactiveStructurePackupEnabled() {
    return getBool("Core3.inactiveStructurePackupEnabled", false);
}

int SRConfigManager::getInactiveStructurePackupDays() {
    return getInt("Core3.inactiveStructurePackupDays", 365);
}

void SRConfigManager::loadCustomConfig() {
    Lua* lua = new Lua();
    lua->init();

    if (!lua->runFile("custom/conf/config.lua")) {
        error("Failed to load custom/conf/config.lua");
        delete lua;
        return;
    }

    setBool("Core3.structurePackupEnabled", lua->getGlobalInt("structurePackupEnabled"));
    setBool("Core3.inactiveStructurePackupEnabled", lua->getGlobalInt("inactiveStructurePackupEnabled"));
    setInt("Core3.inactiveStructurePackupDays", lua->getGlobalInt("inactiveStructurePackupDays"));

    delete lua;
}