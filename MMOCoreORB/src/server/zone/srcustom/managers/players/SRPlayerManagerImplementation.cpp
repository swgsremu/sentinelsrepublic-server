
#include "server/zone/srcustom/managers/players/SRPlayerManager.h"
#include "system/lang/ref/Reference.h"



void SRPlayerManagerImplementation::loadSRLuaConfig() {
    Lua* lua = new Lua();
    lua->init();

    if (!lua->runFile("scripts/custom_scripts/managers/player_manager.lua")) {
        error("Could not load: custom_scripts/managers/player_manager.lua");
        delete lua;
        return;
    }

    // Make sure playerManager is set before using it
    if (playerManager == nullptr) {
        error("PlayerManager reference is null in SRPlayerManager");
        delete lua;
        return;
    }

    LuaObject luaObject = lua->getGlobalObject("srPlayerManager");
    if (!luaObject.isValidTable()) {
        error("Invalid srPlayerManager table in player_manager.lua");
        delete lua;
        return;
    }

    craftingExperienceModifier = luaObject.getFloatField("craftingExperienceModifier");
    globalExperienceModifier = luaObject.getFloatField("globalExpMultiplier"); 
    jediExperienceModifier = luaObject.getFloatField("jediExperienceModifier");

    // Only call setExperienceMultiplier if we have a valid playerManager
    playerManager.get()->setExperienceMultiplier(globalExperienceModifier);

    luaObject.pop();
    delete lua;
}

void SRPlayerManagerImplementation::setCraftingExperience(float experienceModifier) {
	craftingExperienceModifier = experienceModifier;
}

float SRPlayerManagerImplementation::getCraftingExperience() {
	return craftingExperienceModifier;
}