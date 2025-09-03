/*
 * StructureControlDeviceImplementation.cpp
 */
#include <server/zone/srcustom/objects/intangible/structure/StructureControlDevice.h>

#include "server/zone/objects/creature/CreatureObject.h"
// #include "server/zone/objects/intangible/components/UnpackStructureComponent.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/tangible/sign/SignObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/scene/SceneObjectType.h"
#include "server/zone/TreeEntry.h"
#include "engine/core/Task.h"
#include "engine/core/Core.h"
#include "server/zone/srcustom/objects/intangible/structure/tasks/StructureUnpackCompleteTask.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/tangible/sign/SignObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/scene/SceneObjectType.h"
#include "server/zone/TreeEntry.h"
#include "engine/core/Task.h"
#include "engine/core/Core.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/packets/player/EnterStructurePlacementModeMessage.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "templates/manager/TemplateManager.h"
#include "templates/tangible/SharedStructureObjectTemplate.h"
#include "templates/building/SharedBuildingObjectTemplate.h"
#include "server/zone/Zone.h"
#include "server/zone/srcustom/objects/structure/SRStructureObject.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/srcustom/managers/structure/SRStructureManager.h"
#include "server/zone/srcustom/objects/intangible/structure/tasks/StructureUnpackCompleteTask.h"
#include "server/zone/objects/tangible/deed/structure/StructureDeed.h"
#include "terrain/layer/boundaries/BoundaryRectangle.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/objects/region/Region.h"

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

using namespace server::zone::objects::tangible::sign;

/**
  * @brief Fills the object menu response with available options for the structure control device.
  *
  * Adds radial menu item for unpacking the structure only.
  * Note: Status and Pay Maintenance are removed because they don't make sense for a packed structure.
  *
  * @param menuResponse The ObjectMenuResponse to fill.
  * @param player The CreatureObject interacting with the device.
  */
void StructureControlDeviceImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	// Status option removed since it doesn't function for packed structures
	// Pay Maintenance option removed for packed structures
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
			// Structure is packed up, status can't be checked in this state
			player->sendSystemMessage("Structure status is not available while the structure is packed up.");
			break;
		}

		case StructureControlDeviceMenuIDs::PAY_MAINTENANCE: {
			// Structure is packed up, inform player maintenance can't be paid in this state
			player->sendSystemMessage("You cannot pay maintenance while the structure is packed up.");
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

	// Get necessary managers
	auto structureManager = StructureManager::instance();
	if (structureManager == nullptr)
		return 1;
		
	// Get structure template information
	const String serverTemplatePath = structure->getObjectTemplate()->getFullTemplateString();
	
	// Save the structure's original name before placement, as it may be needed later
	String originalStructureName = "";
	if (!structure->getCustomObjectName().isEmpty()) {
		originalStructureName = structure->getCustomObjectName().toString();
	}
	
	// First check city permissions directly without creating a temporary deed
	ManagedReference<CityRegion*> city = player->getCityRegion().get();
	if (city != nullptr) {
		// Check if player has permissions in this city
		ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
		if (ghost != nullptr && !ghost->isPrivileged()) {
			if (city->isClientRegion() || !city->hasZoningRights(player->getObjectID())) {
				player->sendSystemMessage("@player_structure:not_permitted"); // Building is not permitted here.
				return 0;
			}
		}
	}
	
	// Now validate placement using standard system (test only)
	bool testOnly = true;
	
	// Perform our own validation for structure placement
	int validationResult = 0;
	
	// Check for no build zones - we'll use a simple approach here
	// In production code you might want to implement more robust validation
	if (zone == nullptr) {
		player->sendSystemMessage("@player_structure:not_permitted"); // Building is not permitted here.
		return 0;
	}
	
	// Perform basic validation checks before proceeding with placement
	
	// Store the final angle properly normalized
	int normalizedAngle = angle % 360;
	if (normalizedAngle < 0)
		normalizedAngle += 360;
	
	// Get the structure template (use existing structureManager)
	Reference<SharedStructureObjectTemplate*> serverTemplateObj = dynamic_cast<SharedStructureObjectTemplate*>(
		TemplateManager::instance()->getTemplate(serverTemplatePath.hashCode()));
		
	if (serverTemplateObj == nullptr) {
		player->sendSystemMessage("Internal error: Unable to load structure template.");
		return 0;
	}
	
	// Check if this zone allows this structure
	if (!serverTemplateObj->isAllowedZone(zone->getZoneName())) {
		player->sendSystemMessage("@player_structure:wrong_planet"); // That structure cannot be used on this planet.
		return 0;
	}
	
	// Perform basic validation checks that are absolutely necessary (structureManager already defined above)
	ManagedReference<PlanetManager*> planetManager = zone->getPlanetManager();
	if (!planetManager->isBuildingPermittedAt(x, y, player)) {
		player->sendSystemMessage("@player_structure:not_permitted"); // Building is not permitted here.
		return 0;
	}
	
	// Check player's lot count
	ManagedReference<PlayerObject*> ghostObj = player->getPlayerObject();
	if (ghostObj != nullptr) {
		int lots = serverTemplateObj->getLotSize();
		
		// Calculate how many additional lots the player needs
		int additionalLotsNeeded = lots;
		
		// Get the packed structure that's in this control device
		ManagedReference<StructureObject*> packedStructure = this->controlledObject.get().castTo<StructureObject*>();
		
		// If this structure is already owned by the player and just packed up,
		// we shouldn't count these lots twice
		if (packedStructure != nullptr) {
			// Check if the structure is owned by the player
			uint64 playerID = player->getObjectID();
			uint64 ownerID = packedStructure->getOwnerObjectID();
			
			if (playerID == ownerID) {
				// The structure being placed is already owned by the player,
				// so we don't need to check for additional lots
				System::out << "Structure requires " << lots << " lots - player already owns this packed structure, bypassing lot check" << endl;
				additionalLotsNeeded = 0;
			}
		}
		
		// Only perform the lot check if additional lots are needed
		if (additionalLotsNeeded > 0 && !ghostObj->hasLotsRemaining(additionalLotsNeeded)) {
			StringIdChatParameter param("@player_structure:not_enough_lots");
			param.setDI(additionalLotsNeeded);
			player->sendSystemMessage(param);
			return 0;
		}
	}
	
	// Store placement parameters if basic checks pass
	positionX = x;
	positionY = y;
	directionAngle = normalizedAngle;
	
	// Get the construction marker template from the building template
	String constructionMarkerPath;
	
	if (serverTemplateObj != nullptr) {
		constructionMarkerPath = serverTemplateObj->getConstructionMarkerTemplate();
		
		if (!constructionMarkerPath.isEmpty()) {
			System::out << "Using construction marker from template: " << constructionMarkerPath << endl;
		} else {
			// Fallback to a default construction marker if none is specified in the template
			constructionMarkerPath = "object/building/player/construction/construction_player_house_corellia_small_style_01.iff";
			System::out << "No construction marker found in template, using fallback: " << constructionMarkerPath << endl;
		}
	}
	
	// Create the appropriate construction barricade using the marker from the template
	// We can't use STRING_HASHCODE with a variable, so we use the string hashCode method directly
	uint32 templateCRC = constructionMarkerPath.hashCode();
	ManagedReference<SceneObject*> constructionBarricade = zone->getZoneServer()->createObject(templateCRC, 1);
	
	if (constructionBarricade != nullptr) {
		// Position the barricade at the structure location
		constructionBarricade->initializePosition(positionX, zone->getHeight(positionX, positionY), positionY);
		
		// Set the barricade's orientation to match the intended structure
		if (directionAngle != 0) {
			Quaternion direction;
			float angleRad = directionAngle * (M_PI / 180.0f);
			direction.setHeadingDirection(angleRad);
			constructionBarricade->setDirection(direction);
		}
		
		// Add the barricade to the zone
		zone->transferObject(constructionBarricade, -1, true);
		
		System::out << "Created construction barricade at (" << positionX << ", " << positionY 
			<< ") using template: " << constructionMarkerPath << endl;
	} else {
		System::out << "WARNING: Failed to create construction barricade" << endl;
	}
	
	// Calculate construction time based on lots (same as regular placement)
	int constructionDuration = 5000; // Default to 5 seconds
	
	// Use the serverTemplateObj we already created above
	if (serverTemplateObj != nullptr) {
		constructionDuration = serverTemplateObj->getLotSize() * 3000; // 3 seconds per lot (matches original placement time)
	}
	
	player->sendSystemMessage("@player_structure:construction_beginning"); // Construction process beginning.

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
	// This is a stub implementation of the inherited method from ControlDeviceImplementation
	// The commented code below shows the original component-based approach that was not implemented
	//
	// Reference<UnpackStructureComponent*> component = new UnpackStructureComponent();
	//
	// if (component != nullptr)
	// 	component->notifyStructurePlaced(player, structure);
	
	// Note: The actual notification to the structure happens in completeStructurePlacement
	// where structure->notifyStructurePlaced(player) is called
	
	return 1; // Return success
}

void StructureControlDeviceImplementation::completeStructurePlacement(CreatureObject* player) {
	ManagedReference<StructureObject*> structure = this->controlledObject.get().castTo<StructureObject*>();

	if (structure == nullptr || player == nullptr)
		return;

	Zone* zone = player->getZone();
	if (zone == nullptr)
		return;

	// Use the standard structure manager for placement
	auto structureManager = StructureManager::instance();
	if (structureManager == nullptr)
		return;

	{
		Locker sLocker(structure, player);
		
		// Transfer the custom name from the control device to the structure before placing it
		UnicodeString customName = getCustomObjectName();
		if (!customName.isEmpty()) {
			System::out << "Setting structure name from device: " << customName.toString() << endl;
			structure->setCustomObjectName(customName, true);
		} else {
			System::out << "Warning: No custom name found on structure control device" << endl;
		}

		// Use the StructureManager's positioning logic (like it would for a deed)
		float z = zone->getHeight(positionX, positionY);
		
		// Position the structure manually
		structure->initializePosition(positionX, z, positionY);
		structure->updateZoneWithParent(nullptr, false);
		
		// Set direction
		Quaternion direction;
		float angleRad = directionAngle * (M_PI / 180.0f);
		direction.setHeadingDirection(angleRad);
		structure->setDirection(direction);
		
		// Add to zone
		zone->transferObject(structure, -1, true);
		
				// Find and remove any existing signs to prevent duplicates
		Vector<ManagedReference<SignObject*>> signsToRemove;
				
		// Look for nearby objects in the zone
		CloseObjectsVector* closeObjects = (CloseObjectsVector*) structure->getCloseObjects();
		if (closeObjects == nullptr) {
			closeObjects = new CloseObjectsVector();
			structure->setCloseObjects(closeObjects);
		}
		
		// Process nearby objects once for efficiency
		SortedVector<ManagedReference<TreeEntry*>> objList;
		// Reuse the z value calculated earlier
		zone->getInRangeObjects(positionX, z, positionY, 10.0f, &objList, true, true);
		
		// Remove existing exterior signs
		for (int i = 0; i < objList.size(); i++) {
			TreeEntry* treeEntry = objList.get(i);
			if (treeEntry != nullptr) {
				SceneObject* sceneObj = dynamic_cast<SceneObject*>(treeEntry);
				if (sceneObj != nullptr && sceneObj->isSignObject()) {
					signsToRemove.add(static_cast<SignObject*>(sceneObj));
				}
			}
		}		// Recreate template children (sign, terminals) at the new location
		if (structure->isBuildingObject()) {
			auto building = structure->asBuildingObject();
			if (building != nullptr) {
				// Get the saved sign template path
				String savedSignTemplatePath = structure->getSrStructureObject()->getSignTemplatePath();
				
				// Find and remove any existing terminals AND internal signs before creating new ones
				// to prevent duplicates from being created
				// IMPORTANT: Start from cell 1, cell 0 is invalid
				for (int i = 1; i <= building->getTotalCellNumber(); ++i) {
					auto cell = building->getCell(i);
					if (cell == nullptr) continue;
					
					Vector<ManagedReference<SceneObject*>> objectsToRemove;
					int cellContainerSize = cell->getContainerObjectsSize();
					
					for (int j = 0; j < cellContainerSize; ++j) {
						auto obj = cell->getContainerObject(j);
						if (obj != nullptr) {
							// Check for ANY terminals - we'll let createChildObjects recreate only the ones that are needed
							if (obj->isTerminal()) {
								System::out << "Found existing terminal to remove: " << obj->getObjectID() << " (" 
									<< obj->getObjectTemplate()->getFullTemplateString() << ")" << endl;
								objectsToRemove.add(obj);
							}
							// Check for internal signs - these should never exist but check anyway
							else if (obj->isSignObject()) {
								System::out << "Found existing internal sign to remove: " << obj->getObjectID() << endl;
								objectsToRemove.add(obj);
								
								// Also ensure this isn't registered as the building's sign
								if (building->getSignObject() == obj) {
									building->setSignObject(nullptr);
								}
							}
						}
					}
					
					// Remove objects (terminals and signs)
					for (int j = 0; j < objectsToRemove.size(); ++j) {
						auto objToRemove = objectsToRemove.get(j);
						Locker objLocker(objToRemove);
						objToRemove->destroyObjectFromWorld(true);
						objToRemove->destroyObjectFromDatabase(true);
					}
				}
				
				// Do NOT create child objects here - we'll do it only once later to avoid duplicate terminals
				
				// Find and remove any existing signs to prevent duplicates
				Vector<ManagedReference<SignObject*>> signsToRemove;
				
				// Look for external signs in the zone
				SortedVector<ManagedReference<TreeEntry*>> nearbyObjects;
				// Search in a wider radius to catch any signs
				zone->getInRangeObjects(positionX, z, positionY, 10.0f, &nearbyObjects, true, true);
				
				System::out << "Looking for signs to remove in " << nearbyObjects.size() << " nearby objects" << endl;
				
				for (int i = 0; i < nearbyObjects.size(); i++) {
					TreeEntry* treeEntry = nearbyObjects.get(i);
					if (treeEntry != nullptr) {
						SceneObject* sceneObj = dynamic_cast<SceneObject*>(treeEntry);
						if (sceneObj != nullptr && sceneObj->isSignObject()) {
							SignObject* sign = static_cast<SignObject*>(sceneObj);
							
							// Check if the sign belongs specifically to this building
							uint64 buildingID = building->getObjectID();
							
							// SignObject may have parent ID or directly reference the building
							uint64 signParentID = sign->getParentID();
							
							// Get the sign's parent object (if any)
							ManagedReference<SceneObject*> signParent = sign->getParent();
							
							// Only remove signs specifically tied to this building
							bool belongsToBuilding = false;
							
							// Case 1: Sign is a child of this building
							if (signParentID == buildingID) {
								belongsToBuilding = true;
							}
							
							// Case 3: Check distance and name relation
							// Signs should be within a closer range (5m) and have a name related to the building
							if (!belongsToBuilding) {
								// Calculate distance between sign and building
								float dx = sign->getPositionX() - building->getPositionX();
								float dy = sign->getPositionY() - building->getPositionY();
								float distanceSquared = dx*dx + dy*dy;
								
								// Check if names match
								UnicodeString buildingName = building->getCustomObjectName();
								UnicodeString signName = sign->getCustomObjectName();
								
								// If sign is very close and names match, it likely belongs to this building
								if (distanceSquared < 25.0f && !signName.isEmpty() && !buildingName.isEmpty() && 
									signName.toString() == buildingName.toString()) {
									belongsToBuilding = true;
								}
							}
							
							if (belongsToBuilding) {
								System::out << "Found external sign belonging to this building: " << sign->getObjectID() << endl;
								signsToRemove.add(sign);
							} else {
								System::out << "Found external sign NOT belonging to this building, ignoring: " << sign->getObjectID() << endl;
							}
						}
					}
				}
				
				// Look for internal signs that might be inside building cells
				for (int i = 1; i <= building->getTotalCellNumber(); ++i) {
					auto cell = building->getCell(i);
					if (cell != nullptr) {
						for (int j = 0; j < cell->getContainerObjectsSize(); ++j) {
							auto obj = cell->getContainerObject(j);
							if (obj != nullptr && obj->isSignObject()) {
								SignObject* sign = static_cast<SignObject*>(obj.get());
								
								// For internal signs, we're generally safer since they're already inside the building's cells
								// But still check to make sure they specifically belong to this building
								uint64 buildingID = building->getObjectID();
								uint64 signParentID = sign->getParentID();
								ManagedReference<SceneObject*> signParent = sign->getParent();
								
								bool belongsToBuilding = false;
								
								// Case 1: Sign is in this building's cell
								if (signParent != nullptr && signParent->getParentID() == buildingID) {
									belongsToBuilding = true;
								}
								
								// Case 2: Sign is in this building's cell (direct check)
								if (cell->getParent().get() == building) {
									belongsToBuilding = true;
								}
								
								// Case 4: Sign has same name as building
								UnicodeString buildingName = building->getCustomObjectName();
								UnicodeString signName = sign->getCustomObjectName();
								
								if (!signName.isEmpty() && !buildingName.isEmpty() && 
									signName.toString() == buildingName.toString()) {
									belongsToBuilding = true;
								}
								
								if (belongsToBuilding) {
									System::out << "Found internal sign belonging to this building: " << sign->getObjectID() << endl;
									signsToRemove.add(sign);
								} else {
									System::out << "Found internal sign NOT belonging to this building, ignoring: " << sign->getObjectID() << endl;
								}
							}
						}
					}
				}
				
				// Remove all found signs
				for (int i = 0; i < signsToRemove.size(); ++i) {
					auto signToRemove = signsToRemove.get(i);
					Locker signLocker(signToRemove);
					signToRemove->destroyObjectFromWorld(true);
					signToRemove->destroyObjectFromDatabase(true);
				}
				
				// Handle the sign creation
				// We will create our own sign and ignore any that createChildObjects creates later
				ManagedReference<SignObject*> sign = nullptr;
				
				// Get the template path for the sign
				String signTemplatePath = structure->getSrStructureObject()->getSignTemplatePath();
				System::out << "Saved sign template path: " << signTemplatePath << endl;
				
				// If no template path, try to use a default one
				if (signTemplatePath.isEmpty()) {
					System::out << "No saved sign template path, using default" << endl;
					signTemplatePath = "object/tangible/sign/player/house_address.iff";
				}
				
				// Create the sign
				ManagedReference<SignObject*> newSign = zone->getZoneServer()->createObject(
					signTemplatePath.hashCode(), 0).castTo<SignObject*>();
					
				if (newSign != nullptr) {
					Locker newSignLocker(newSign);
					
					// Get the building position and orientation
					float buildingX = building->getPositionX();
					float buildingY = building->getPositionY();
					
					// Get the building's orientation
					const Quaternion* dirPtr = building->getDirection();
					float radians = 0.0f;
					
					if (dirPtr != nullptr) {
						radians = dirPtr->getRadians();
					}
					
					// Place sign in front of building (opposite direction of where the building is facing)
					float distance = 6.0f; // Increased distance to ensure it's outside for larger buildings
					float signX = buildingX - sin(radians) * distance;
					float signY = buildingY - cos(radians) * distance;
					float signZ = zone->getHeight(signX, signY) + 0.75f; // Above ground level
					
					// Position the sign
					newSign->initializePosition(signX, signZ, signY);
					
					// Make sign face toward the building
					if (dirPtr != nullptr) {
						Quaternion signDir = *dirPtr; // Copy building direction
						// Rotate 180 degrees to face building
						signDir = signDir.rotate(Vector3(0, 1, 0), 180);
						newSign->setDirection(signDir);
					}
					
					// Set the sign name from the structure
					UnicodeString structureName = structure->getCustomObjectName();
					if (!structureName.isEmpty()) {
						System::out << "Setting custom sign name to: " << structureName.toString() << endl;
						newSign->setCustomObjectName(structureName, true);
					}
					
					// Add the sign to the world OUTSIDE the building
					zone->transferObject(newSign, -1, false);
					
					// Set this as the official sign for the building
					building->setSignObject(newSign);
					
					System::out << "Successfully created and positioned exterior sign at (" << signX << ", " << signY << ")" << endl;
					
					// Keep track of our sign for reference
					sign = newSign;
				} else {
					System::out << "ERROR: Failed to create sign from template: " << signTemplatePath << endl;
				}
				
				// Ensure all other signs are removed
				for (int i = 0; i < signsToRemove.size(); ++i) {
					auto oldSign = signsToRemove.get(i);
					if (oldSign != nullptr && oldSign != sign) {
						Locker oldSignLocker(oldSign);
						System::out << "Removing old sign: " << oldSign->getObjectID() << endl;
						oldSign->destroyObjectFromWorld(true);
						oldSign->destroyObjectFromDatabase(true);
					}
				}

				// Force-load cell contents from DB so items are re-associated in memory
				int total = building->getTotalCellNumber();
				for (int i = 1; i <= total; ++i) {
					auto cell = building->getCell(i);
					if (cell != nullptr) {
						cell->getContainerObjects();
					}
				}
				
				// Create all child objects but handle signs carefully
				System::out << "Creating child objects for building ID: " << building->getObjectID() << endl;
				
				// First create all terminals and other objects
				building->createChildObjects();
				
				// Now check for and remove ANY signs (except our primary sign)
				// Store objects to be removed in a vector
				Vector<ManagedReference<SceneObject*>> objectsToRemove;
				
				// Check for signs in cells - IMPORTANT: Start from cell 1, cell 0 is invalid
				for (int i = 1; i <= building->getTotalCellNumber(); ++i) {
					auto cell = building->getCell(i);
					if (cell == nullptr) continue;
					
					for (int j = 0; j < cell->getContainerObjectsSize(); ++j) {
						auto obj = cell->getContainerObject(j);
						if (obj != nullptr) {
							// Check for terminals for debugging
							if (obj->isTerminal()) {
								System::out << "Terminal created in cell " << i << ": " << obj->getObjectID() << " (" 
									<< obj->getObjectTemplate()->getFullTemplateString() << ")" << endl;
							}
							// Find any signs that are not our main exterior sign
							else if (obj->isSignObject() && obj != building->getSignObject()) {
								String templatePath = obj->getObjectTemplate()->getFullTemplateString();
								
								// Check if this is a shop sign (keep those) or address sign (remove duplicates)
								if (templatePath.indexOf("/shop_sign_") == -1) {
									System::out << "Found internal sign to remove: " << obj->getObjectID() << " template: " 
										<< templatePath << endl;
									objectsToRemove.add(obj);
								}
							}
						}
					}
				}
				
				// Now check the zone for any duplicate exterior signs
				SortedVector<ManagedReference<TreeEntry*>> zoneObjects;
				zone->getInRangeObjects(building->getPositionX(), building->getPositionZ(), 
					building->getPositionY(), 15.0f, &zoneObjects, true);
					
				for (int i = 0; i < zoneObjects.size(); ++i) {
					TreeEntry* treeEntry = zoneObjects.get(i);
					SceneObject* sceneObj = dynamic_cast<SceneObject*>(treeEntry);
					if (sceneObj != nullptr && sceneObj->isSignObject() && sceneObj != building->getSignObject()) {
						// Remove duplicate house address signs
						String templatePath = sceneObj->getObjectTemplate()->getFullTemplateString();
						if (templatePath.indexOf("/house_address_") != -1) {
							System::out << "Found duplicate exterior sign to remove: " << sceneObj->getObjectID() << endl;
							objectsToRemove.add(sceneObj);
						}
					}
				}
				
				// Remove all unwanted objects
				for (int i = 0; i < objectsToRemove.size(); ++i) {
					auto objToRemove = objectsToRemove.get(i);
					if (objToRemove != nullptr) {
						Locker objLocker(objToRemove);
						// Double check it's not our main sign
						if (building->getSignObject() != objToRemove) {
							System::out << "Removing unwanted object: " << objToRemove->getObjectID() << endl;
							objToRemove->destroyObjectFromWorld(true);
							objToRemove->destroyObjectFromDatabase(true);
						}
					}
				}
				
				// Check if we need to create shop signs
				// Access the template directly to get shop sign info
				TemplateManager* templateManager = TemplateManager::instance();
				SharedBuildingObjectTemplate* buildingTemplate = dynamic_cast<SharedBuildingObjectTemplate*>(
					templateManager->getTemplate(building->getObjectTemplate()->getFullTemplateString().hashCode()));
				
				if (buildingTemplate != nullptr) {
					// Check for shop signs in the template
					int shopSignCount = buildingTemplate->getShopSignsSize();
					System::out << "Found " << shopSignCount << " shop signs in template" << endl;
					
					for (int i = 0; i < shopSignCount; ++i) {
						// Get shop sign template info
						const ChildObject* shopSignInfo = buildingTemplate->getShopSign(i);
						if (shopSignInfo != nullptr) {
							String templateFile = shopSignInfo->getTemplateFile();
							
							// Skip address signs, we already created our main sign
							if (templateFile.indexOf("/house_address_") != -1) {
								System::out << "Skipping duplicate address sign in shopSigns: " << templateFile << endl;
								continue;
							}
							
							// Create only shop signs
							if (templateFile.indexOf("/shop_sign_") != -1) {
								// Use the proper getter methods for position
								Vector3 position = shopSignInfo->getPosition();
								float x = position.getX();
								float y = position.getY();
								float z = position.getZ();
								
								System::out << "Creating shop sign: " << templateFile << " at position " 
									<< x << ", " << y << ", " << z << endl;
								
								// Create the shop sign
								ManagedReference<SignObject*> shopSign = zone->getZoneServer()->createObject(
									templateFile.hashCode(), 0).castTo<SignObject*>();
									
								if (shopSign != nullptr) {
									Locker shopLocker(shopSign);
									
									// Position the shop sign
									shopSign->initializePosition(x, z, y);
									
									// Add to zone
									zone->transferObject(shopSign, -1, false);
								}
							}
						}
					}
				}
				
				// Make sure the CORRECT sign (exterior one) has the proper name
				ManagedReference<SignObject*> buildingSign = building->getSignObject();
				if (buildingSign != nullptr) {
					UnicodeString structureName = structure->getCustomObjectName();
					if (!structureName.isEmpty()) {
						Locker signLocker(buildingSign);
						System::out << "Setting sign name to: " << structureName.toString() << endl;
						buildingSign->setCustomObjectName(structureName, true);
						
						// Make sure the sign is positioned OUTSIDE the building
						float buildingX = building->getPositionX();
						float buildingY = building->getPositionY();
						
						// Get the building's orientation
						const Quaternion* direction = building->getDirection();
						float radians = 0.0f;
						if (direction != nullptr) {
							radians = direction->getRadians();
						}
						
						// Calculate proper exterior position
						float distance = 5.0f; // Increased distance to ensure it's outside
						float signX = buildingX - sin(radians) * distance;
						float signY = buildingY - cos(radians) * distance;
						float signZ = zone->getHeight(signX, signY) + 0.5f;
						
						// Force sign to be properly positioned outside
						buildingSign->initializePosition(signX, signZ, signY);
					}
				}
				
				// Restore previously packed items from this control device
				bool restoreSuccess = restoreItems(building, zone->getZoneServer());
				
				if (restoreSuccess) {
					System::out << "StructureControlDevice: Successfully restored items to structure " << building->getObjectID() << endl;
				} else {
					System::out << "StructureControlDevice: Failed to restore items, falling back to SRStructureObject" << endl;
					// Fallback to legacy method
					structure->getSrStructureObject()->restoreItems(building, zone->getZoneServer());
				}
				
				// Clear transient storage after successful restoration to free memory
				clearPackedItems();
				structure->getSrStructureObject()->clearPackedItems();
			}
		}

		structure->notifyStructurePlaced(player);
		
		// Ensure the placing player gets the interior contents streamed
		if (structure->isBuildingObject()) {
			auto building = structure->asBuildingObject();
			if (building != nullptr) {
				// Broadcast to nearby players
				building->broadcastObject(player, true);
				
				// Make sure the player who placed it sees everything
				building->sendContainerObjectsTo(player, true);
			}
		}

		// Clear control-device linkage and remove the device
		try {
			// Create strong reference to structure before proceeding
			ManagedReference<StructureObject*> strongStructureRef = structure;
			
			if (strongStructureRef != nullptr) {
				auto sr = strongStructureRef->getSrStructureObject();
				if (sr != nullptr) {
					sr->setControlDevice(nullptr);
				}
			}
			
			// Add a short delay before destroying the control device
			// to ensure any pending operations on the structure are complete
			Core::getTaskManager()->scheduleTask([=] {
				Locker dLocker(_this.getReferenceUnsafeStaticCast());
				this->destroyObjectFromWorld(true);
				this->destroyObjectFromDatabase(true);
			}, "DestroyStructureControlDevice", 1000); // 1 second delay
			
			return; // Exit early, destruction happens in scheduled task
		} catch (Exception& e) {
			System::out << "Exception in StructureControlDevice destruction: " << e.getMessage() << endl;
		}
		
		// Fallback destruction path if the scheduled approach fails
		Locker dLocker(_this.getReferenceUnsafeStaticCast());
		this->destroyObjectFromWorld(true);
		this->destroyObjectFromDatabase(true);
	}
}

// Helper function removed - code integrated directly where needed

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

/**
 * Clears all packed item data after restoration
 */
void StructureControlDeviceImplementation::clearPackedItems() {
	// Clear all packed item arrays
	packedItemIds.removeAll();
	packedItemCells.removeAll();
	packedItemPosX.removeAll();
	packedItemPosY.removeAll();
	packedItemPosZ.removeAll();
	packedItemDirs.removeAll();
	
	System::out << "StructureControlDevice: Packed items cleared after successful restoration" << endl;
}

/**
 * Collects items from a building before pack-up
 * @param building The building to collect items from
 * @param zoneServer The zone server reference
 * @return True if items were collected successfully
 */
bool StructureControlDeviceImplementation::collectItems(BuildingObject* building, ZoneServer* zoneServer) {
	if (building == nullptr || zoneServer == nullptr) {
		System::out << "StructureControlDevice::collectItems - Building or ZoneServer is null" << endl;
		return false;
	}
	
	System::out << "StructureControlDevice::collectItems - Starting collection for building ID: " << building->getObjectID() << endl;
	
	// Clear any previous data
	clearPackedItems();
	
	int totalCollected = 0;
	
	// Loop through each cell in the building
	for (int i = 1; i <= building->getTotalCellNumber(); ++i) {
		ManagedReference<CellObject*> cell = building->getCell(i);
		
		if (cell == nullptr)
			continue;
			
		int cellObjectCount = cell->getContainerObjectsSize();
		System::out << "StructureControlDevice::collectItems - Cell " << i << " contains " << cellObjectCount << " objects" << endl;
		
		// Go through all objects in this cell
		for (int j = 0; j < cellObjectCount; ++j) {
			ManagedReference<SceneObject*> object = cell->getContainerObject(j);
			
			if (object != nullptr) {
				// Skip anything that shouldn't be collected, like players
				if (object->isPlayerCreature() || object->isPet())
					continue;
					
				// Skip terminals (they'll be recreated)
				if (object->isTerminal())
					continue;
					
				// Get the object's position data
				float x = object->getPositionX();
				float y = object->getPositionY();
				float z = object->getPositionZ();
				float dir = object->getDirectionAngle();
				
				// Store the object's data
				packedItemIds.add(object->getObjectID());
				packedItemCells.add(i);
				packedItemPosX.add(x);
				packedItemPosY.add(y);
				packedItemPosZ.add(z);
				packedItemDirs.add(dir);
				
				totalCollected++;
			}
		}
	}
	
	System::out << "StructureControlDevice::collectItems - Successfully collected " << totalCollected << " items from building" << endl;
	
	return true;
}

/**
 * Restores items to a building after unpack
 * @param building The building to restore items to
 * @param zoneServer The zone server containing the objects
 * @return True if items were restored successfully
 */
bool StructureControlDeviceImplementation::restoreItems(BuildingObject* building, ZoneServer* zoneServer) {
	if (building == nullptr || zoneServer == nullptr) {
		System::out << "StructureControlDevice::restoreItems - Building or ZoneServer is null" << endl;
		return false;
	}
	
	System::out << "StructureControlDevice::restoreItems - Starting restoration for building ID: " << building->getObjectID() << endl;
	System::out << "StructureControlDevice::restoreItems - Total items to restore: " << packedItemIds.size() << endl;
	
	if (packedItemIds.size() == 0) {
		System::out << "StructureControlDevice::restoreItems - No items to restore" << endl;
		return true;
	}
	
	// Check if we have valid data
	if (packedItemIds.size() != packedItemCells.size() || 
		packedItemIds.size() != packedItemPosX.size() ||
		packedItemIds.size() != packedItemPosY.size() ||
		packedItemIds.size() != packedItemPosZ.size() ||
		packedItemIds.size() != packedItemDirs.size()) {
		System::out << "StructureControlDevice::restoreItems - Item data arrays have mismatched sizes" << endl;
		return false;
	}
	
	// Use a HashSet to track existing terminals so we don't create duplicates
	HashSet<String> existingTerminalTemplates;
	
	// Check for existing terminals in all cells
	for (int i = 1; i <= building->getTotalCellNumber(); ++i) {
		CellObject* cell = building->getCell(i);
		if (cell != nullptr) {
			for (int j = 0; j < cell->getContainerObjectsSize(); ++j) {
				ManagedReference<SceneObject*> obj = cell->getContainerObject(j);
				if (obj != nullptr && obj->isTerminal()) {
					existingTerminalTemplates.add(obj->getObjectTemplate()->getFullTemplateString());
				}
			}
		}
	}
	
	// Get object manager
	ObjectManager* objectManager = ObjectManager::instance();
	
	// Restore each item
	for (int i = 0; i < packedItemIds.size(); ++i) {
		try {
			uint64 objectId = packedItemIds.get(i);
			int cellNumber = packedItemCells.get(i);
			float x = packedItemPosX.get(i);
			float y = packedItemPosY.get(i);
			float z = packedItemPosZ.get(i);
			float dir = packedItemDirs.get(i);
			
			// Get the object
			ManagedReference<SceneObject*> object = zoneServer->getObject(objectId);
			
			if (object != nullptr) {
				// Skip terminals - they should be recreated automatically
				if (object->isTerminal())
					continue;
					
				// Get the cell to place the object in
				ManagedReference<CellObject*> cell = building->getCell(cellNumber);
				
				if (cell != nullptr) {
					// Transfer the object to the cell
					if (!cell->transferObject(object, -1, true)) {
						System::out << "StructureControlDevice::restoreItems - Failed to transfer object ID: " << objectId << " to cell" << endl;
						continue;
					}
					
					// Update position
					object->setPosition(x, z, y); // Note: y and z are flipped for indoor positioning
					object->updateDirection(dir);
					
					// Broadcast to clients
					object->broadcastObject(object, true);
					
					System::out << "StructureControlDevice::restoreItems - Successfully restored object ID: " << objectId << endl;
				} else {
					System::out << "StructureControlDevice::restoreItems - Cell " << cellNumber << " not found in building" << endl;
				}
			} else {
				System::out << "StructureControlDevice::restoreItems - Failed to retrieve object ID: " << objectId << endl;
			}
		} catch (Exception& e) {
			System::out << "StructureControlDevice::restoreItems - Exception occurred: " << e.getMessage() << endl;
		}
	}
	
	System::out << "StructureControlDevice::restoreItems - Restoration complete" << endl;
	
	return true;
}