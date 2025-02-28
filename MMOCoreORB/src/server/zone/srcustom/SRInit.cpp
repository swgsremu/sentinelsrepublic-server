#include "SRInit.h"

void SRPreInitialize() {
    // Initialize custom ConfigManager to read custom configs
    const auto srConfigManager = new SRConfigManager();
    ConfigManager::setSingletonInstance(srConfigManager);
}

void SRPostInitialize() {
}