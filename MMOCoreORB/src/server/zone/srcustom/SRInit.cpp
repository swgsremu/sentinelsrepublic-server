#include "SRInit.h"
#include "server/zone/srcustom/managers/configuration/SRConfigManager.h"

void SRPreInitialize() {
    // Initialize custom ConfigManager to read custom configs
    const auto srConfigManager = new SRConfigManager();
    ConfigManager::setSingletonInstance(srConfigManager);
}

void SRPostInitialize() {
}