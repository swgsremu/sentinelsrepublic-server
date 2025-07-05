/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
*/

#include "LairSpawnAreaUtils.h"
#include "server/zone/objects/tangible/TangibleObject.h"
#include "server/zone/objects/area/ActiveArea.h"
#include "server/zone/objects/area/areashapes/CircularAreaShape.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/objects/area/events/RemoveNoSpawnAreaTask.h"

void LairSpawnAreaUtils::createNoSpawnArea(TangibleObject* lair) {
	if (lair == nullptr) {
		return;
	}

	// Lock the lair to safely read its properties
	Locker lairLocker(lair);

	auto zone = lair->getZone();
	if (zone == nullptr) {
		return;
	}

	auto zoneServer = zone->getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}

	// Cache position values while we have the lock
	float posX = lair->getPositionX();
	float posY = lair->getPositionY();

	// Release the lair lock before creating new objects
	lairLocker.release();

	ManagedReference<CircularAreaShape*> areaShape = new CircularAreaShape();
	Locker alocker(areaShape);
	
	areaShape->setRadius(64);
	areaShape->setAreaCenter(posX, posY);
	
	ManagedReference<ActiveArea*> noSpawnArea = (zoneServer->createObject(STRING_HASHCODE("object/active_area.iff"), 0)).castTo<ActiveArea*>();
	
	if (noSpawnArea != nullptr) {
		Locker locker(noSpawnArea);
		
		noSpawnArea->initializePosition(posX, 0, posY);
		noSpawnArea->setAreaShape(areaShape);
		noSpawnArea->addAreaFlag(ActiveArea::NOSPAWNAREA);
		
		// Re-check zone validity before transfer
		if (zone->getZoneServer() != nullptr && zone->transferObject(noSpawnArea, -1, true)) {
			// Schedule removal of no-spawn area after 30 seconds
			Reference<RemoveNoSpawnAreaTask*> removeTask = new RemoveNoSpawnAreaTask(noSpawnArea);
			if (removeTask != nullptr) {
				removeTask->schedule(30000);
			}
		}
	}
}