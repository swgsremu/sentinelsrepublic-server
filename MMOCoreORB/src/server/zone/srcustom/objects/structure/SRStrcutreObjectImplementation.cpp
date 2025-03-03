
#include <server/zone/objects/pathfinding/NavArea.h>
#include <server/zone/objects/player/PlayerObject.h>
#include "server/zone/srcustom/objects/structure/SRStructureObject.h"


bool SRStructureObjectImplementation::unloadFromZone(bool sendSelfDestroy) {
	ManagedReference<Zone*> zone = getZone();

	if (zone == nullptr)
		return false;

	// ManagedReference<BuildingObject*> building = cast<BuildingObject*>(asSceneObject());

	// if (building == nullptr)
	// 	return false;
	//
	// // ManagedReference<SceneObject*> owner = zone->getZoneServer()->getObject(getOwnerObjectID());
	//
	// if (owner == nullptr)
	// 	return false;
	//
	// ManagedReference<SceneObject*> ghost = owner->getSlottedObject("ghost");

	// if (ghost == nullptr || !ghost->isPlayerObject())
	// 	return false;
	//
	// if (navArea != nullptr) {
	// 	ManagedReference<NavArea*> nav = navArea;
	// 	Core::getTaskManager()->executeTask([nav, sendSelfDestroy] () {
	// 		Locker locker(nav);
	// 		nav->destroyObjectFromWorld(sendSelfDestroy);
	// 	}, "destroyStructureNavAreaLambda2");
	// }
	//
	// PlayerObject* playerObject = cast<PlayerObject*>(ghost.get());
	//
	// if (getObjectID() == playerObject->getDeclaredResidence())
	// 	playerObject->setDeclaredResidence(nullptr);
	//
	// uint64 waypointID = getWaypointID();
	//
	// if (waypointID != 0)
	// 	playerObject->removeWaypoint(waypointID, true, true);
	//
	// float x = getPositionX();
	// float y = getPositionY();
	// float z = zone->getHeight(x, y);
	//
	// building->destroyChildObjects();
	//
	// for (uint32 i = 1; i <= building->getTotalCellNumber(); ++i) {
	// 	ManagedReference<CellObject*> cellObject = building->getCell(i);
	//
	// 	if (cellObject == nullptr)
	// 		continue;
	//
	// 	int childObjects = cellObject->getContainerObjectsSize();
	//
	// 	if (childObjects <= 0)
	// 		continue;
	//
	// 	for (int j = childObjects - 1; j >= 0; --j) {
	// 		ManagedReference<SceneObject*> containedObject = cellObject->getContainerObject(j);
	//
	// 		if (containedObject->isPlayerCreature() || containedObject->isPet()) {
	// 			CreatureObject* creature = cast<CreatureObject*>(containedObject.get());
	// 			creature->teleport(x, z, y, 0);
	// 			building->onExit(creature, 0);
	// 			continue;
	// 		}
	// 	}
	// }
	//
	// removeObjectFromZone(zone, asSceneObject());
	// setZone(nullptr);
	// scheduleMaintenanceExpirationEvent();

	return true;
}