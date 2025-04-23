
#include "server/zone/srcustom/managers/players/SRPlayerManager.h"



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


    craftingExperienceModifier = lua->getGlobalFloat("craftingExpModifier");
    globalExperienceModifier = lua->getGlobalFloat("globalExpMultiplier"); 
    jediExperienceModifier = lua->getGlobalFloat("jediExpMultiplier");

    // Only call setExperienceMultiplier if we have a valid playerManager
    playerManager.get()->setExperienceMultiplier(globalExperienceModifier);

    delete lua;
}

void SRPlayerManagerImplementation::setCraftingExperience(float experienceModifier) {
	craftingExperienceModifier = experienceModifier;
}

float SRPlayerManagerImplementation::getCraftingExperience() {
	return craftingExperienceModifier;
}

void SRPlayerManagerImplementation::setGroupExperienceModifier(float groupExpMultiplier) {
}
