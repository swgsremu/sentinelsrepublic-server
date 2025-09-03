#include "SRStructureManager.h"

#include <server/zone/ZoneServer.h>
#include <server/zone/Zone.h>
#include <server/zone/objects/building/BuildingObject.h>
#include <server/zone/objects/cell/CellObject.h>
#include <server/zone/objects/tangible/sign/SignObject.h>
#include <server/zone/srcustom/objects/player/sessions/PackupStructureSession.h>
#include "server/zone/srcustom/objects/intangible/structure/StructureControlDevice.h"
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

            // Get the control device to collect items
            ManagedReference<StructureControlDevice*> structureControlDevice = cast<StructureControlDevice*>(controlDevice.get());
            
            if (structureControlDevice != nullptr) {
                System::out << "Collecting items for structure using StructureControlDevice..." << endl;
                structureControlDevice->collectItems(building, server);
            } else {
                // Fallback to old method
                System::out << "Using fallback method to collect items..." << endl;
                structureObject->getSrStructureObject()->collectItems(building);
            }

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
                // First save the main sign's template before removing it
                SignObject* sign = building->getSignObject();
                if (sign != nullptr) {
                    // Save sign template information before destroying it
                    structureObject->getSrStructureObject()->saveSignInfo(sign);
                    
                    Locker signLock(sign);
                    sign->destroyObjectFromWorld(true);
                    sign->destroyObjectFromDatabase(true);
                }
                
                // Also search for any other signs that might be around or in the building
                Vector<ManagedReference<SignObject*>> extraSignsToRemove;
                
                // Check cells for any lingering signs
                // IMPORTANT: Start from cell 1, cell 0 is invalid
                for (int i = 1; i <= building->getTotalCellNumber(); ++i) {
                    ManagedReference<CellObject*> cell = building->getCell(i);
                    if (cell == nullptr) continue;
                    
                    for (int j = 0; j < cell->getContainerObjectsSize(); ++j) {
                        ManagedReference<SceneObject*> obj = cell->getContainerObject(j);
                        if (obj != nullptr && obj->isSignObject()) {
                            extraSignsToRemove.add(static_cast<SignObject*>(obj.get()));
                        }
                    }
                }
                
                // Check for any external signs near the building too
                SortedVector<ManagedReference<TreeEntry*>> nearbyObjects;
                Zone* zone = building->getZone();
                if (zone != nullptr) {
                    zone->getInRangeObjects(building->getPositionX(), building->getPositionZ(), 
                                      building->getPositionY(), 10.0f, &nearbyObjects, true, true);
                    
                    for (int i = 0; i < nearbyObjects.size(); i++) {
                        TreeEntry* treeEntry = nearbyObjects.get(i);
                        if (treeEntry != nullptr) {
                            SceneObject* obj = dynamic_cast<SceneObject*>(treeEntry);
                            if (obj != nullptr && obj != sign && obj->isSignObject()) {
                                extraSignsToRemove.add(static_cast<SignObject*>(obj));
                            }
                        }
                    }
                }
                
                // Now destroy any extra signs we found
                for (int i = 0; i < extraSignsToRemove.size(); ++i) {
                    SignObject* extraSign = extraSignsToRemove.get(i);
                    if (extraSign != nullptr) {
                        Locker extraSignLocker(extraSign);
                        extraSign->destroyObjectFromWorld(true);
                        extraSign->destroyObjectFromDatabase(true);
                    }
                }
            }

            if (building->getCustomObjectName() != "")
                controlDevice->setCustomObjectName(building->getCustomObjectName(), true);
            else
                controlDevice->setCustomObjectName(structureObject->getDisplayedName(), true);

            // Note: We don't store orientation since players can choose their own orientation when unpacking
            
            Locker deviceLocker(controlDevice, structureObject);
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