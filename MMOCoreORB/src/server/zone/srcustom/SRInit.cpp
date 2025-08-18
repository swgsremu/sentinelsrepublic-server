#include "SRInit.h"
#include "server/zone/srcustom/managers/configuration/SRConfigManager.h"
#include "server/zone/managers/components/ComponentManager.h"
#include "server/zone/srcustom/objects/tangible/terminal/components/SRStructureTerminalMenuComponent.h"

void SRPreInitialize() {
    // Initialize custom ConfigManager to read custom configs
    const auto srConfigManager = new SRConfigManager();
    ConfigManager::setSingletonInstance(srConfigManager);
}

void SRPostInitialize() {
}

void SRRegisterComponents() {
    // Override the default StructureTerminalMenuComponent with our SR variant
    ComponentManager::instance()->putComponent("StructureTerminalMenuComponent", new SRStructureTerminalMenuComponent());
    Logger::console.info("SR: Registered SRStructureTerminalMenuComponent override", true);
}