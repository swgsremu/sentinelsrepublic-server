#include "SRConfigManager.h"

SRConfigManager::SRConfigManager() {
	loadConfigData();	// Load base config
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
	} else {
		error() << "SRConfig table not found in conf/custom/config.lua";
	}
	srConfig.pop();

	delete lua;
}