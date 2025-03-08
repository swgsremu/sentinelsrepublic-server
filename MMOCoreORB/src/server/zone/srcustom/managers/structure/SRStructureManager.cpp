#include "SRStructureManager.h"

#include <server/zone/ZoneServer.h>
#include <server/zone/objects/building/BuildingObject.h>
#include <server/zone/srcustom/objects/player/sessions/PackupStructureSession.h>



void SRStructureManager::setStructureManager(StructureManager* manager) {
	structureManager = manager;
}

int SRStructureManager::packupStructure(CreatureObject* creature) {
	const ManagedReference<PackupStructureSession*> session = creature->getActiveSession(SRSessionFacadeType::PACKUPSTRUCTURE).castTo<PackupStructureSession*>();
	const auto server = creature->getZoneServer();
	if (session == nullptr)
		return 0;

	ManagedReference<StructureObject*> structureObject = session->getStructureObject();

	if (structureObject == nullptr)
		return 0;

	Locker _locker(structureObject);

	const int maint = structureObject->getSurplusMaintenance();
	const int redeedCost = structureObject->getRedeedCost();

	// if (structureObject->isRedeedable()) {
		// ManagedReference<StructureControlDevice*> controlDevice = server->createObject(STRING_HASHCODE("object/intangible/house/generic_house_control_device.iff"), 1).castTo<StructureControlDevice*>();

		// if (controlDevice == nullptr)
		// 	return session->cancelSession();
		//
		// Locker _lock(controlDevice, structureObject);
		//
		// const ManagedReference<SceneObject*> datapad = creature->getSlottedObject("datapad");
		//
		// if (datapad == nullptr)
		// 	return session->cancelSession();
		//
		// if (datapad->isContainerFullRecursive()) {
		// 	creature->sendSystemMessage("Structure Packup Failed: Your datapad is full!");
		// 	controlDevice->destroyObjectFromWorld(true);
		// 	controlDevice->destroyObjectFromDatabase(true);
		// 	return session->cancelSession();
		// } else {
		// 	ManagedReference<BuildingObject*> building = cast<BuildingObject*>(structureObject.get());
		//
		// 	if (building == nullptr || !structureObject->getSrStructureObject()->unloadFromZone(true)) {
		// 		creature->sendSystemMessage("Structure Packup Failed! Building was null or it couldn't be unloaded from zone. Please open a support ticket reporting this error.");
		// 		controlDevice->destroyObjectFromWorld(true);
		// 		controlDevice->destroyObjectFromDatabase(true);
		// 		return session->cancelSession();
		// 	}
		//
		// 	if (building->getCustomObjectName() != "")
		// 		controlDevice->setCustomObjectName(building->getCustomObjectName(), true);
		// 	else
		// 		controlDevice->setCustomObjectName(structureObject->getDisplayedName(), true);
		//
		// 	controlDevice->setControlledObject(structureObject);
		// 	controlDevice->updateStatus(1);
		//
		// 	structureObject->setSurplusMaintenance(maint - redeedCost);
		// 	structureObject->getSrStructureObject()->setControlDevice(controlDevice);
		//
		// 	datapad->transferObject(controlDevice, -1);
		// 	datapad->broadcastObject(controlDevice, true);

	// 		StringBuffer msg;
	// 		msg << "Structure ID: " << structureObject->getObjectID() << " Owned By: " << creature->getFirstName() << " has been packed up by the player.";
	//
	// 		// structurePackupLog.info(msg.toString());
	// 		creature->sendSystemMessage("Structure Pack Up Successful! A structure control device has been placed in your datapad.");
	// 	}
	// }

	return session->cancelSession();
	return 0;
}