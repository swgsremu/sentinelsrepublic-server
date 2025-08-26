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
		}

		// Complete the structure placement
		deviceRef->completeStructurePlacement(playerRef);
	}
};

#endif /*STRUCTUREUNPACKCOMPLETETASK_H_*/
