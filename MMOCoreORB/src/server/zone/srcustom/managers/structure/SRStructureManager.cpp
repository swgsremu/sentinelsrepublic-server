#include "SRStructureManager.h"

#include <server/zone/ZoneServer.h>
#include <server/zone/objects/building/BuildingObject.h>
#include <server/zone/srcustom/objects/player/sessions/PackupStructureSession.h>
#include "server/zone/srcustom/objects/intangible/structure/StructureControlDevice.h"


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

/**
 * @brief Packs up a structure and places a control device in the player's datapad.
 *
 * This method encapsulates the process of packing up a player-owned structure. It retrieves
 * the structure object from the active packup session, creates a control device representing
 * the packed structure, and places this device into the player's datapad. The method also
 * handles the transfer of maintenance fees and redeed costs associated with the structure.
 *
 * The method performs several checks to ensure the packup process is valid, including:
 *   - Verifying the existence of an active packup session.
 *   - Ensuring the structure object is valid.
 *   - Confirming the structure is redeedable.
 *   - Validating the player's datapad has sufficient space.
 *   - Successfully unloading the structure from the game zone.
 *
 * If any of these checks fail, the method sends an appropriate system message to the player
 * and cancels the packup session.
 *
 * @param creature The creature initiating the packup. This is the player character who owns the structure.
 * @return 0 if the packup was successful and the control device was placed in the player's datapad.
 *         Returns the result of `session->cancelSession()` if the packup process fails at any point.
 */
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
        ManagedReference<StructureControlDevice*> controlDevice = server->createObject(CONTROL_DEVICE_HASH, 1).castTo<StructureControlDevice*>();

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

            if (building == nullptr || !structureObject->getSrStructureObject()->unloadFromZone(true)) {
                creature->sendSystemMessage(BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE);
                controlDevice->destroyObjectFromWorld(true);
                controlDevice->destroyObjectFromDatabase(true);
                return session->cancelSession();
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