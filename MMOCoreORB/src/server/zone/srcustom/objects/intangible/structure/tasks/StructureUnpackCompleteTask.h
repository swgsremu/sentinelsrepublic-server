/*
 * StructureUnpackCompleteTask.h
 *
 * Task to handle completion of structure unpacking after construction delay
 */

#ifndef STRUCTUREUNPACKCOMPLETETASK_H_
#define STRUCTUREUNPACKCOMPLETETASK_H_

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/srcustom/objects/intangible/structure/StructureControlDevice.h"
#include "server/zone/objects/scene/SceneObject.h"

class StructureUnpackCompleteTask : public Task {
	ManagedWeakReference<CreatureObject*> player;
	ManagedWeakReference<StructureControlDevice*> controlDevice;
	ManagedWeakReference<SceneObject*> constructionBarricade;

public:
	StructureUnpackCompleteTask(CreatureObject* playerRef, StructureControlDevice* deviceRef, SceneObject* barricadeRef) : Task() {
		player = playerRef;
		controlDevice = deviceRef;
		constructionBarricade = barricadeRef;
	}

	void run() {
		ManagedReference<CreatureObject*> playerRef = player.get();
		ManagedReference<StructureControlDevice*> deviceRef = controlDevice.get();

		if (playerRef == nullptr || deviceRef == nullptr)
			return;

		Locker lock(playerRef);

		// Remove construction barricade
		ManagedReference<SceneObject*> barricadeRef = constructionBarricade.get();
		if (barricadeRef != nullptr) {
			Locker barricadeLocker(barricadeRef);
			barricadeRef->destroyObjectFromWorld(true);
			System::out << "Removed construction barricade" << endl;
		}

		// Send message to player with proper parameters
		ManagedReference<TangibleObject*> controlledObject = deviceRef->getControlledObject();
		if (controlledObject != nullptr && controlledObject->isStructureObject()) {
			ManagedReference<StructureObject*> structure = cast<StructureObject*>(controlledObject.get());
			if (structure != nullptr && playerRef->getPlayerObject() != nullptr) {
				StringIdChatParameter message("@player_structure:construction_complete");
				message.setTO(structure->getObjectName());
				message.setDI(playerRef->getPlayerObject()->getLotsRemaining());
				playerRef->sendSystemMessage(message);
			} else {
				// Fallback to simple message if we can't get the structure or player object
				playerRef->sendSystemMessage("@player_structure:construction_complete");
			}
		} else {
			// Fallback to simple message if we can't get the controlled object
			playerRef->sendSystemMessage("@player_structure:construction_complete");
		}

		// Complete the structure placement
		deviceRef->completeStructurePlacement(playerRef);
	}
};

#endif /*STRUCTUREUNPACKCOMPLETETASK_H_*/
