#include "SRInit.h"

void SRPreInitialize() {
    // Initialize custom ConfigManager to read custom configs
    const auto srConfigManager = new SRConfigManager();
    ConfigManager::setSingletonInstance(srConfigManager);
}

void SRPostInitialize() {
  //   ZoneServer* zoneServer = ServerCore::getZoneServer();
  //   std::cout << zoneServer << std::endl;
  //   if (zoneServer != nullptr) {
		// const auto srCommandManager = new SRCommandConfigManager(zoneServer);
		// const auto objectController = zoneServer->getObjectController();
  //       if (objectController != nullptr) {
  //           srCommandManager->loadSlashCommandsFile();
  //       }
  //   }
}