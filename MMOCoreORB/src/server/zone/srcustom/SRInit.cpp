#include "SRInit.h"

void SRInitialize() {
    // Initialize custom ConfigManager to read custom configs
    SRConfigManager* srConfigManager = new SRConfigManager();
    ConfigManager::setSingletonInstance(srConfigManager);
}