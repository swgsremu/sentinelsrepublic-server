/*
 * FishingPoleObjectImplementation.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: swgemu
 */

#include "server/zone/objects/tangible/fishing/FishingPoleObject.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/scene/TransferErrorCode.h"
#include "server/zone/ZoneProcessServer.h"
#include "server/zone/managers/minigames/FishingManager.h"
#include "server/zone/objects/creature/CreatureObject.h"

void FishingPoleObjectImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	TangibleObjectImplementation::fillAttributeList(alm,object);

	alm->insertAttribute("quality", String::valueOf(quality));
}


int FishingPoleObjectImplementation::canAddObject(SceneObject* object, int containmentType, String& errorDescription) {
	if (object->isFishingBait()) {
		return TangibleObjectImplementation::canAddObject(object, containmentType, errorDescription);
	}

	errorDescription = "@fishing:bait_only";

	return TransferErrorCode::BAITONLY;
}

void FishingPoleObjectImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	TangibleObjectImplementation::fillObjectMenuResponse(menuResponse, player);
	String text = getText(player);
	menuResponse->addRadialMenuItem(245, 3, text);
	/*text = "@fishing:mnu_open_tackle";
	menuResponse.addRadialMenuItem(20, 3, text);*/
}

int FishingPoleObjectImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	if (selectedID == 245) {
		doFishing(player);
	}

	return 0;
}

void FishingPoleObjectImplementation::updateCraftingValues(CraftingValues* values, bool firstUpdate) {
	// useModifer is the quality
	quality = values->getCurrentValue("usemodifier");
}

void FishingPoleObjectImplementation::doFishing(CreatureObject* player) {
	auto fishingManager = server->getFishingManager();

	if (fishingManager == nullptr) {
		return;
	}

	if (fishingManager->getFishingState(player) != FishingManager::NOTFISHING) {
		fishingManager->stopFishing(player, fishingManager->getFishBoxID(player), true);
	} else {
		player->executeObjectControllerAction(STRING_HASHCODE("fish"));
	}
}

String FishingPoleObjectImplementation::getText(CreatureObject* player) {
	ManagedReference<FishingManager*> manager = server->getFishingManager();
	String text = "@fishing:mnu_start_fishing";

	if (manager->getFishingState(player) != FishingManager::NOTFISHING) {
		text = "@fishing:mnu_stop_fishing";
	}

	return text;
}

bool FishingPoleObjectImplementation::removeObject(SceneObject* object, SceneObject* destination, bool notifyClient, bool nullifyParent) {
	ManagedReference<FishingManager*> manager = server->getFishingManager();
	ManagedReference<SceneObject*> strongParent = getParent().get();
	if (strongParent == nullptr)
		return false;

	if ((strongParent != nullptr) && (strongParent->isPlayerCreature())) {
		ManagedReference<CreatureObject*> player = cast<CreatureObject*>(strongParent.get());
		if ((player != nullptr) && (object->isFishingBait())) {
			if (manager->getFishingState(player) != FishingManager::NOTFISHING) {
				player->sendSystemMessage("Cannot remove bait while fishing pole is in use.");

				return false;
			}

			return TangibleObjectImplementation::removeObject(object, destination, notifyClient);
		}
	}
	return false;
}
