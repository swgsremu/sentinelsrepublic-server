/*
 * SRStructureTerminalMenuComponent.cpp
 */

#include "SRStructureTerminalMenuComponent.h"

#include "server/zone/Zone.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/objects/tangible/terminal/Terminal.h"
#include "server/zone/managers/structure/StructureManager.h"
#include "server/zone/objects/player/sessions/StructureSetAccessFeeSession.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/managers/creature/PetManager.h"

// SR Custom: Packup Structure
#include "server/zone/srcustom/managers/configuration/SRConfigManager.h"
#include "server/zone/srcustom/objects/player/sessions/PackupStructureSession.h"
#include "server/zone/srcustom/objects/scene/SRSessionFacadeType.h"
#include "server/zone/srcustom/objects/structure/SRStructureObject.h"

namespace SRRadialIds {
constexpr byte PACK_UP_STRUCTURE = 132; // add under management
}

void SRStructureTerminalMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* creature) const {
    // Call base to add standard items
    StructureTerminalMenuComponent::fillObjectMenuResponse(sceneObject, menuResponse, creature);

    if (!SRConfigManager::instance()->getStructurePackupEnabled())
        return;

    if(!sceneObject->isTerminal() || !creature->isPlayerCreature())
        return;

    auto terminal = cast<Terminal*>(sceneObject);
    if (terminal == nullptr)
        return;

    auto structureObject = cast<StructureObject*>(terminal->getControlledObject());
    if (structureObject == nullptr)
        return;

    if (!structureObject->isOnAdminList(creature))
        return;

    // Add our custom Pack Up item under the management radial (118)
    menuResponse->addRadialMenuItemToRadialID(118, SRRadialIds::PACK_UP_STRUCTURE, 3, "@faction_perk:packup");
}

int SRStructureTerminalMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* creature, byte selectedID) const {
    error() << "SRStructureTerminalMenuComponent: handleObjectMenuSelect called with selectedID: " << (int)selectedID;
    
    if (selectedID != SRRadialIds::PACK_UP_STRUCTURE) {
        error() << "SRStructureTerminalMenuComponent: selectedID " << (int)selectedID << " != PACK_UP_STRUCTURE " << (int)SRRadialIds::PACK_UP_STRUCTURE << ", calling base handler";
        return StructureTerminalMenuComponent::handleObjectMenuSelect(sceneObject, creature, selectedID);
    }

    error() << "SRStructureTerminalMenuComponent: Pack up structure selected by player: " << creature->getFirstName();

    auto terminal = cast<Terminal*>(sceneObject);
    if(terminal == nullptr || !creature->isPlayerCreature())
        return 1;

    auto structureObject = cast<StructureObject*>(terminal->getControlledObject());
    if (structureObject == nullptr)
        return 1;

    if (structureObject->getZone() == nullptr)
        return 1;

    if (creature->containsActiveSession(SRSessionFacadeType::PACKUPSTRUCTURE))
        return 0;

    auto ghost = creature->getPlayerObject();
    if (ghost == nullptr)
        return 0;

    if (!ghost->isOwnedStructure(structureObject) && !ghost->isStaff()) {
        creature->sendSystemMessage("@player_structure:packup_must_be_owner");
        return 0;
    }

    if (!SRConfigManager::instance()->getStructurePackupEnabled()) {
        creature->sendSystemMessage("@player_structure:packup_not_eligible_01");
        return 0;
    }

    const auto message = SRStructureObject::getPackupMessage();
    if (!message.isEmpty()) {
        creature->sendSystemMessage(String("@player_structure") + message);
        return 0;
    }

    ManagedReference<PackupStructureSession*> session = new PackupStructureSession(creature, structureObject);
    session->initializeSession();
    return 0;
}


