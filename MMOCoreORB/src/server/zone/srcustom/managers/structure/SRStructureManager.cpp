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

const uint32 SRStructureManager::CONTROL_DEVICE_HASH = STRING_HASHCODE("object/intangible/house/shared_generic_house_control_device.iff");


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
    error() << "Packup: packupStructure method called for player: " << creature->getFirstName();
    
    const ManagedReference<PackupStructureSession*> session = creature->getActiveSession(SRSessionFacadeType::PACKUPSTRUCTURE).castTo<PackupStructureSession*>();
    const auto server = creature->getZoneServer();
    if (session == nullptr) { 
        error() << "Packup: No active PackupStructureSession found for player: " << creature->getFirstName();
        return 0;
    }

    error() << "Packup: PackupStructureSession found, proceeding with packup";
    ManagedReference<StructureObject*> structureObject = session->getStructureObject();

    if (structureObject == nullptr) {
        error() << "Packup: StructureObject is null";
        return 0;
    }

    Locker _locker(structureObject);

    const int maint = structureObject->getSurplusMaintenance();
    const int redeedCost = structureObject->getRedeedCost();

    error() << "Packup: Structure maintenance: " << maint << ", redeed cost: " << redeedCost;
    error() << "Packup: Checking if structure is redeedable...";
    
    if (structureObject->isRedeedable()) {
        error() << "Packup: Structure IS redeedable, proceeding with control device creation";
        info() << "Packup: Creating control device with template: " << CONTROL_DEVICE_HASH;
        ManagedReference<ControlDevice*> controlDevice = server->createObject(CONTROL_DEVICE_HASH, 1).castTo<ControlDevice*>();

        if (controlDevice == nullptr) {
            error() << "Packup: Failed to create control device from template";
            return session->cancelSession();
        }

        info() << "Packup: Control device created successfully with ObjectID: " << controlDevice->getObjectID()
               << ", GameObjectType: " << controlDevice->getGameObjectType();

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
            info() << "Packup: Datapad found - Container size: " << datapad->getContainerObjectsSize() 
                   << "/" << datapad->getContainerVolumeLimit() 
                   << ", isContainerFull: " << datapad->isContainerFullRecursive();

            ManagedReference<BuildingObject*> building = cast<BuildingObject*>(structureObject.get());

            if (building == nullptr) {
                creature->sendSystemMessage(BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE);
                controlDevice->destroyObjectFromWorld(true);
                controlDevice->destroyObjectFromDatabase(true);
                return session->cancelSession();
            }

            structureObject->getSrStructureObject()->collectItems(building);

            {
                Locker buildingLock(building, creature);
                if (creature->getParent() != nullptr && creature->getRootParent() == building) {
                    Vector3 ep = building->getEjectionPoint();
                    creature->teleport(ep.getX(), ep.getZ(), ep.getY(), 0);
                }
                if (building->getZone() != nullptr) {
                    building->destroyObjectFromWorld(true);
                }
            }

            if (building->getZone() != nullptr) {
                creature->sendSystemMessage(BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE);
                controlDevice->destroyObjectFromWorld(true);
                controlDevice->destroyObjectFromDatabase(true);
                return session->cancelSession();
            }

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

            info() << "Packup: Datapad container size before transfer: " << datapad->getContainerObjectsSize();
            info() << "Packup: Control device created with ID: " << controlDevice->getObjectID();
            bool transferResult = datapad->transferObject(controlDevice, -1);
            if (transferResult) {
                datapad->broadcastObject(controlDevice, true);
                info() << "Packup: Control device successfully transferred to datapad";
            } else {
                error() << "Packup: Failed to transfer control device to datapad";
                controlDevice->destroyObjectFromWorld(true);
                controlDevice->destroyObjectFromDatabase(true);
                creature->sendSystemMessage("Structure Packup Failed: Could not add control device to datapad.");
                return session->cancelSession();
            }

            StringBuffer msg;
            msg << LOG_PREFIX << structureObject->getObjectID() << LOG_OWNED_BY << creature->getFirstName() << LOG_SUFFIX;

            creature->sendSystemMessage(SUCCESS_MESSAGE);
            return session->cancelSession();
        }
    } else {
        error() << "Packup: Structure is NOT redeedable - maintenance: " << maint << ", required: " << redeedCost;
        creature->sendSystemMessage("@player_structure:packup_items_maint");
        return session->cancelSession();
    }
}