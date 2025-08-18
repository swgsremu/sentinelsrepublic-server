#include "SRStructureManager.h"

#include <server/zone/ZoneServer.h>
#include <server/zone/objects/building/BuildingObject.h>
#include <server/zone/objects/tangible/sign/SignObject.h>
#include <server/zone/srcustom/objects/player/sessions/PackupStructureSession.h>
#include "server/zone/objects/intangible/ControlDevice.h"
#include "engine/util/u3d/Vector3.h"


const String SRStructureManager::DATAPAD_FULL_MESSAGE = "Structure Packup Failed: Your datapad is full!";
const String SRStructureManager::BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE = "Structure Packup Failed! Building was null or it couldn't be unloaded from zone. Please open a support ticket reporting this error.";
const String SRStructureManager::SUCCESS_MESSAGE = "Structure Pack Up Successful! A structure control device has been placed in your datapad.";

const String SRStructureManager::LOG_PREFIX = "Structure ID: ";
const String SRStructureManager::LOG_OWNED_BY = " Owned By: ";
const String SRStructureManager::LOG_SUFFIX = " has been packed up by the player.";

const uint32 SRStructureManager::CONTROL_DEVICE_HASH = STRING_HASHCODE("object/intangible/house/generic_house_control_device.iff");


/**
 * @brief Sets the structure manager instance.
 *
 * This method sets the structure manager instance used by this class.
 *
 * @param manager The structure manager instance.
 */
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

    if (structureObject->isRedeedable()) {
        ManagedReference<ControlDevice*> controlDevice = server->createObject(CONTROL_DEVICE_HASH, 1).castTo<ControlDevice*>();

        if (controlDevice == nullptr)
            return session->cancelSession();

        Locker _lock(controlDevice, structureObject);

        const ManagedReference<SceneObject*> datapad = creature->getSlottedObject("datapad");

        if (datapad == nullptr)
            return session->cancelSession();

        if (datapad->isContainerFullRecursive()) {
            creature->sendSystemMessage(DATAPAD_FULL_MESSAGE);
            controlDevice->destroyObjectFromWorld(true);
            controlDevice->destroyObjectFromDatabase(true);
            return session->cancelSession();
        } else {
            ManagedReference<BuildingObject*> building = cast<BuildingObject*>(structureObject.get());

            // Actually unload the structure from the world by destroying it from the zone
            if (building == nullptr) {
                creature->sendSystemMessage(BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE);
                controlDevice->destroyObjectFromWorld(true);
                controlDevice->destroyObjectFromDatabase(true);
                return session->cancelSession();
            }

            // SR: snapshot item OIDs per cell before world removal (transient only)
            structureObject->getSrStructureObject()->collectItems(building);

            {
                Locker buildingLock(building, creature);

                // If the player is inside this building, move them to the building ejection point first
                if (creature->getParent() != nullptr && creature->getRootParent() == building) {
                    Vector3 ep = building->getEjectionPoint();
                    creature->teleport(ep.getX(), ep.getZ(), ep.getY(), 0);
                }

                if (building->getZone() != nullptr) {
                    // Remove from world but keep database entry for redeed
                    building->destroyObjectFromWorld(true);
                }
            }

            // Validate it is no longer in a zone
            if (building->getZone() != nullptr) {
                creature->sendSystemMessage(BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE);
                controlDevice->destroyObjectFromWorld(true);
                controlDevice->destroyObjectFromDatabase(true);
                return session->cancelSession();
            }

            // Remove exterior sign from database so it can be cleanly recreated at the new location on unpack
            {
                SignObject* sign = building->getSignObject();
                if (sign != nullptr) {
                    Locker signLock(sign);
                    sign->destroyObjectFromWorld(true);
                    sign->destroyObjectFromDatabase(true);
                }
            }

            if (building->getCustomObjectName() != "")
                controlDevice->setCustomObjectName(building->getCustomObjectName(), true);
            else
                controlDevice->setCustomObjectName(structureObject->getDisplayedName(), true);

            controlDevice->setControlledObject(structureObject);
            controlDevice->updateStatus(1);

            structureObject->setSurplusMaintenance(maint - redeedCost);
            structureObject->getSrStructureObject()->setControlDevice(controlDevice);

            datapad->transferObject(controlDevice, -1);
            datapad->broadcastObject(controlDevice, true);

            StringBuffer msg;
            msg << LOG_PREFIX << structureObject->getObjectID() << LOG_OWNED_BY << creature->getFirstName() << LOG_SUFFIX;

            creature->sendSystemMessage(SUCCESS_MESSAGE);
        }
    }

    return session->cancelSession();
}