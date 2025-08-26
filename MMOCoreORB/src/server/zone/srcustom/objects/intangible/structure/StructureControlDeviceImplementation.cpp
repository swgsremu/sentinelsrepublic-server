/*
	 	 * StructureControlDeviceImplementation.cpp
	 	 */
#include <server/zone/srcustom/objects/intangible/structure/StructureControlDevice.h>

#include "server/zone/objects/creature/CreatureObject.h"
// #include "server/zone/objects/intangible/components/UnpackStructureComponent.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/packets/player/EnterStructurePlacementModeMessage.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "templates/manager/TemplateManager.h"
#include "templates/tangible/SharedStructureObjectTemplate.h"
#include "server/zone/Zone.h"
#include "server/zone/srcustom/objects/structure/SRStructureObject.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/srcustom/managers/structure/SRStructureManager.h"
#include "server/zone/srcustom/objects/intangible/structure/tasks/StructureUnpackCompleteTask.h"

namespace StructureControlDeviceMenuIDs {
constexpr byte STATUS = 18;
constexpr byte PAY_MAINTENANCE = 19;
constexpr byte UNPACK_STRUCTURE = 20;
}

namespace SystemMessages {
const String CANT_PLACE_MOUNTED = "@player_structure:cant_place_mounted";
const String NOT_INSIDE = "@player_structure:not_inside";
const String NOT_PERMITTED = "@player_structure:not_permitted";
const String MANAGEMENT_STATUS = "@player_structure:management_status";
const String MANAGEMENT_PAY = "@player_structure:management_pay";
const String STRUCTURE_UNPACK = "@player_structure:structure_unpack";
}

/**
  * @brief Fills the object menu response with available options for the structure control device.
  *
  * Adds radial menu items for status, paying maintenance, and unpacking the structure.
  *
  * @param menuResponse The ObjectMenuResponse to fill.
  * @param player The CreatureObject interacting with the device.
  */
void StructureControlDeviceImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	menuResponse->addRadialMenuItem(StructureControlDeviceMenuIDs::STATUS, 3, SystemMessages::MANAGEMENT_STATUS);
	menuResponse->addRadialMenuItem(StructureControlDeviceMenuIDs::PAY_MAINTENANCE, 3, SystemMessages::MANAGEMENT_PAY);
	menuResponse->addRadialMenuItem(StructureControlDeviceMenuIDs::UNPACK_STRUCTURE, 3, SystemMessages::STRUCTURE_UNPACK);
}

/**
  * @brief Handles the selection of an option from the object menu.
  *
  * Executes actions based on the selected menu item, such as displaying the structure status,
  * paying maintenance, or initiating the structure placement mode.
  *
  * @param player The CreatureObject interacting with the device.
  * @param selectedID The ID of the selected menu item.
  * @return int Returns 0 if handled, otherwise returns an error code.
  */
int StructureControlDeviceImplementation::handleObjectMenuSelect(CreatureObject* player, const byte selectedID) {
	ManagedReference<StructureObject*> structure = this->controlledObject.get().castTo<StructureObject*>();

	if (structure == nullptr || !isASubChildOf(player))
		return 0;

	switch (selectedID) {
		case StructureControlDeviceMenuIDs::STATUS: {
			player->executeObjectControllerAction(0x13F7E585, structure->getObjectID(), ""); //structureStatus
			break;
		}

		case StructureControlDeviceMenuIDs::PAY_MAINTENANCE: {
			player->executeObjectControllerAction(0xE7E35B30, structure->getObjectID(), ""); //payMaintenance
			break;
		}

		case StructureControlDeviceMenuIDs::UNPACK_STRUCTURE: {
			placeStructureMode(player, structure);
			break;
		}

		default:
			return 0;
	}

	return 0;
}

/**
  * @brief Initiates the structure placement mode for the player.
  *
  * Sends a message to the client to enter structure placement mode, allowing the player to
  * position the structure for placement.
  *
  * @param player The CreatureObject placing the structure.
  * @param structure The StructureObject to be placed.
  */
void StructureControlDeviceImplementation::placeStructureMode(CreatureObject* player, StructureObject* structure) {
	if (player->isRidingMount()) {
		player->sendSystemMessage(SystemMessages::CANT_PLACE_MOUNTED);
		return;
	}

	if (player->getParent() != nullptr) {
		player->sendSystemMessage(SystemMessages::NOT_INSIDE); //You can not place a structure while you are inside a building.
		return;
	}

	ManagedReference<CityRegion*> city = player->getCityRegion().get();

	if (city != nullptr && city->isClientRegion()) {
		player->sendSystemMessage(SystemMessages::NOT_PERMITTED); //Building is not permitted here.
		return;
	}

	const TemplateManager* templateManager = TemplateManager::instance();
	const String serverTemplatePath = structure->getObjectTemplate()->getFullTemplateString();
	Reference<SharedStructureObjectTemplate*> serverTemplate = dynamic_cast<SharedStructureObjectTemplate*>(templateManager->getTemplate(serverTemplatePath.hashCode()));

	if (serverTemplate == nullptr)
		return;

	const String clientTemplatePath = templateManager->getTemplateFile(serverTemplate->getClientObjectCRC());
	const auto espmm = new EnterStructurePlacementModeMessage(this->getObjectID(), clientTemplatePath);

	player->sendMessage(espmm);
}

/**
  * @brief Places the structure at the specified coordinates and angle.
  *
  * @param player The CreatureObject placing the structure.
  * @param x The x-coordinate for placement.
  * @param y The y-coordinate for placement.
  * @param angle The angle of rotation for placement.
  * @return int Returns 1 if placement was initiated, otherwise returns an error code.
  */
int StructureControlDeviceImplementation::placeStructure(CreatureObject* player, float x, float y, int angle) {
	ManagedReference<StructureObject*> structure = this->controlledObject.get().castTo<StructureObject*>();

	if (structure == nullptr)
		return 1;

	Zone* zone = player->getZone();
	if (zone == nullptr)
		return 1;

	// Get template information for construction barricade
	const TemplateManager* templateManager = TemplateManager::instance();
	const String serverTemplatePath = structure->getObjectTemplate()->getFullTemplateString();
	Reference<SharedStructureObjectTemplate*> serverTemplate = dynamic_cast<SharedStructureObjectTemplate*>(templateManager->getTemplate(serverTemplatePath.hashCode()));

	if (serverTemplate == nullptr)
		return 1;

	// Create construction barricade for "under construction" effect
	String barricadeServerTemplatePath = serverTemplate->getConstructionMarkerTemplate();
	int constructionDuration = 100; // Default fallback duration

	ManagedReference<SceneObject*> constructionBarricade = nullptr;
	if (!barricadeServerTemplatePath.isEmpty()) {
		constructionBarricade = ObjectManager::instance()->createObject(barricadeServerTemplatePath.hashCode(), 0, "");

		if (constructionBarricade != nullptr) {
			float z = zone->getHeight(x, y);
			constructionBarricade->initializePosition(x, z, y);

			const StructureFootprint* structureFootprint = serverTemplate->getStructureFootprint();
			int barricadeAngle = angle;

			if (structureFootprint != nullptr && (structureFootprint->getRowSize() > structureFootprint->getColSize())) {
				barricadeAngle = angle + 180;
			}

			constructionBarricade->rotate(barricadeAngle);

			Locker bLocker(constructionBarricade);
			zone->transferObject(constructionBarricade, -1, true);

			constructionDuration = serverTemplate->getLotSize() * 3000; // 3 seconds per lot
		}
	}

	// Store placement parameters for delayed construction completion
	positionX = x;
	positionY = y;
	directionAngle = angle;

	// Schedule construction completion task
	Reference<Task*> task = new StructureUnpackCompleteTask(player, _this.getReferenceUnsafeStaticCast(), constructionBarricade);
	task->schedule(constructionDuration);

	return 1;
}

/**
  * @brief Notifies the player that the structure has been placed.
  *
  * Placeholder for any post-placement notifications or actions.
  *
  * @param player The CreatureObject who placed the structure.
  * @param structure The StructureObject that was placed.
  * @return int Returns 1 if notification was sent, otherwise returns an error code.
  */
int StructureControlDeviceImplementation::notifyStructurePlaced(CreatureObject* player, StructureObject* structure) {
	// Reference<UnpackStructureComponent*> component = new UnpackStructureComponent();
	//
	// if (component != nullptr)
	// 	component->notifyStructurePlaced(player, structure);

	return 1;
}

void StructureControlDeviceImplementation::completeStructurePlacement(CreatureObject* player) {
	ManagedReference<StructureObject*> structure = this->controlledObject.get().castTo<StructureObject*>();

	if (structure == nullptr || player == nullptr)
		return;

	Zone* zone = player->getZone();
	if (zone == nullptr)
		return;

	{
		Locker sLocker(structure, player);

		float z = zone->getHeight(positionX, positionY);
		structure->initializePosition(positionX, z, positionY);
		structure->rotate(directionAngle);

		zone->transferObject(structure, -1, true);

		// Recreate template children (sign, terminals) at the new location
		if (structure->isBuildingObject()) {
			auto building = structure->asBuildingObject();
			if (building != nullptr) {
				structure->createChildObjects();

				// Force-load cell contents from DB so items are re-associated in memory
				int total = building->getTotalCellNumber();
				for (int i = 1; i <= total; ++i) {
					auto cell = building->getCell(i);
					if (cell != nullptr) {
						cell->getContainerObjects();
					}
				}
				// Restore previously packed items (transient) if any
				structure->getSrStructureObject()->restoreItems(building, zone->getZoneServer());
			}
		}

		structure->notifyStructurePlaced(player);

		// Ensure the placing player gets the interior contents streamed
		if (structure->isBuildingObject()) {
			auto building = structure->asBuildingObject();
			if (building != nullptr) {
				building->sendContainerObjectsTo(player, true);
			}
		}
	}

	// Clear control-device linkage and remove the device
	{
		auto sr = structure->getSrStructureObject();
		if (sr)
			sr->setControlDevice(nullptr);

		Locker dLocker(_this.getReferenceUnsafeStaticCast());
		this->destroyObjectFromWorld(true);
		this->destroyObjectFromDatabase(true);
	}
}

void StructureControlDeviceImplementation::storeObject(CreatureObject* player, bool force) {
    // For structure control device, storing means re-dedeeding the structure, which this device does not support.
    // Silently ignore to avoid abstract-method errors from generic tasks attempting to store all control devices.
}

/**
  * @brief Fills the attribute list with structure-specific attributes.
  *
  * Adds attributes such as allowed zones for placement to the attribute list displayed to the player.
  *
  * @param alm The AttributeListMessage to fill.
  * @param object The CreatureObject viewing the attributes.
  */
void StructureControlDeviceImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	SceneObjectImplementation::fillAttributeList(alm, object);

	ManagedReference<StructureObject*> structure = this->controlledObject.get().castTo<StructureObject*>();

	if (structure == nullptr)
		return;

	const TemplateManager* templateManager = TemplateManager::instance();
	const uint32 structureCRC = structure->getObjectTemplate()->getFullTemplateString().hashCode();
	const auto structureTemplate = dynamic_cast<SharedStructureObjectTemplate*>(templateManager->getTemplate(structureCRC));

	if (structureTemplate == nullptr)
		return;

	for (int i = 0; i < structureTemplate->getTotalAllowedZones(); ++i) {
		String zoneName = structureTemplate->getAllowedZone(i);

		if (!zoneName.isEmpty())
			alm->insertAttribute("examine_scene", "@planet_n:" + zoneName); //Can Be Built On
	}
}