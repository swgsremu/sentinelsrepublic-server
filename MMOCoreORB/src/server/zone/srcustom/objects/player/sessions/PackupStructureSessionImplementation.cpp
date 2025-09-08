/*
 * PackupStructureSessionImplementation.cpp
 */

#include <server/zone/managers/structure/StructureManager.h>
#include <functional>

#include "server/zone/srcustom/objects/player/sessions/PackupStructureSession.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/Zone.h"
#include "server/zone/srcustom/objects/scene/SRSessionFacadeType.h" 

namespace PackupStructureConstants {
constexpr int MIN_PACKUP_CODE = 100000;
constexpr int MAX_PACKUP_CODE = 999999;
}

int PackupStructureSessionImplementation::initializeSession() {
	info() << "PackupStructureSession: initializeSession called for player: " << creatureObject->getFirstName();
	
	if (!creatureObject->isPlayerCreature()) {
		error() << "PackupStructureSession: Player is not a creature, canceling session";
		return cancelSession();
	}

	creatureObject->addActiveSession(SRSessionFacadeType::PACKUPSTRUCTURE, _this.getReferenceUnsafeStaticCast());
	info() << "PackupStructureSession: Active session added successfully";
	
	Locker structureLock(structureObject, creatureObject);

	CreatureObject* player = creatureObject.get();
	const String redeed = structureObject->isRedeedable() ? "\\#32CD32 @player_structure:can_redeed_yes_suffix \\#." : "\\#FF6347 @player_structure:can_redeed_no_suffix \\#.";
	
	// Initialize variables to track contents
	int itemCount = 0;
	int vendorCount = 0;
	bool hasVendors = false;
	
	if (structureObject->isBuildingObject()) {
		// Structure to track both vendors and items in a single pass
		struct BuildingContentsInfo {
			int itemCount = 0;
			int vendorCount = 0;
			bool hasVendors = false;
		};
		
		// Initialize our tracking structure
		BuildingContentsInfo contents;
		BuildingObject* building = structureObject->asBuildingObject();
		if (building != nullptr) {
			info() << "PackupStructureSession: Scanning building: " << building->getObjectID();
			int totalCells = building->getTotalCellNumber();
			
			// Helper function to recursively process objects - handles both vendor detection and item counting
			std::function<int(SceneObject*)> processObjectRecursive;
			
			processObjectRecursive = [&](SceneObject* obj) -> int {
				if (obj == nullptr)
					return 0;
				
				// Check if this is a vendor first
				if (obj->isVendor()) {
					contents.hasVendors = true;
					contents.vendorCount++;
					info() << "PackupStructureSession: Found vendor: " << obj->getDisplayedName();
					return 0; // Vendors aren't counted as items
				}
				
				// Skip creatures and signs
				if (obj->isCreatureObject() || obj->isSignObject())
					return 0;
				
				// Get the template path once for all checks
				String templatePath = obj->getObjectTemplate()->getFullTemplateString();
				
				// We need this reference for checking child objects
				ManagedReference<SceneObject*> strongParent = building;
				
				// Special handling for terminals - don't skip veteran rewards
				if (obj->isTerminal()) {
						// Include veteran reward terminals
						if (templatePath.indexOf("veteran_reward/data_terminal") != -1) {
							debug() << "PackupStructureSession: Including veteran reward terminal: " << templatePath;
						} else {
							debug() << "PackupStructureSession: Skipping regular terminal: " << templatePath;
							return 0;
						}
					}
					
					// Make sure we're counting backpacks themselves
					if (templatePath.indexOf("backpack") != -1) {
						debug() << "PackupStructureSession: Found backpack: " << templatePath;
					}
					
					// Skip default house items that will be automatically recreated
					
					// Check for deed items - count these as items
					if (templatePath.indexOf("deed") != -1) {
						// Structure deeds should be counted as items
						debug() << "PackupStructureSession: Including deed: " << templatePath;
						return 1; // Count as 1 item
					}
					
					// Only skip specific terminals that are part of the house structure
					// Don't skip custom terminals or veteran rewards
					if ((templatePath.indexOf("terminal") != -1 && 
						(templatePath.indexOf("structure_terminal") != -1 || 
						 templatePath.indexOf("house_control_terminal") != -1)) || 
						templatePath.indexOf("structure_storage_") != -1 ||
						templatePath.indexOf("house_") != -1) {
						debug() << "PackupStructureSession: Skipping default item: " << templatePath;
						return 0;
					}
					
					// Don't skip veteran reward data terminals
					if (templatePath.indexOf("data_terminal") != -1) {
						debug() << "PackupStructureSession: Including data terminal: " << templatePath;
					}
					
					if (strongParent->containsChildObject(obj)) {
						debug() << "PackupStructureSession: Skipping child object: " << templatePath;
						return 0;
					}
					
					debug() << "PackupStructureSession: Counting item: " << templatePath;
					
					// Always count this object as 1 item
					int count = 1;
					
					// If this is a container, count both the container AND its contents separately
					if (obj->isContainerObject()) {
						debug() << "PackupStructureSession: Object is a container, counting both container and contents";
						// Debug log the container
						debug() << "PackupStructureSession: Container: " << obj->getDisplayedName() << " counts as 1 item";
						
						// Count items inside the container separately
						for (int k = 0; k < obj->getContainerObjectsSize(); ++k) {
							ManagedReference<SceneObject*> containerItem = obj->getContainerObject(k);
							int itemCount = processObjectRecursive(containerItem);
							count += itemCount;
							debug() << "PackupStructureSession: Container item: " << (containerItem != nullptr ? containerItem->getDisplayedName() : "null") << " adds " << itemCount << " to total";
						}
					} else if (obj->isCraftingStation()) {
						// Special handling for crafting stations (similar to how structure status counts)
						ManagedReference<SceneObject*> hopper = obj->getSlottedObject("ingredient_hopper");
						if (hopper != nullptr) {
							debug() << "PackupStructureSession: Crafting station hopper found";
							for (int k = 0; k < hopper->getContainerObjectsSize(); ++k) {
								ManagedReference<SceneObject*> hopperItem = hopper->getContainerObject(k);
								int itemCount = processObjectRecursive(hopperItem);
								count += itemCount;
								debug() << "PackupStructureSession: Hopper item: " << (hopperItem != nullptr ? hopperItem->getDisplayedName() : "null") << " adds " << itemCount << " to total";
							}
						}
					}
					
					return count;
				}; // End of lambda function
			
			// Single pass through all cells to detect vendors and count items
			for (int i = 1; i <= totalCells; ++i) {
				CellObject* cell = building->getCell(i);
				if (cell == nullptr)
					continue;
				
				info() << "PackupStructureSession: Processing cell " << i << ", containing " << cell->getContainerObjectsSize() << " objects";
				for (int j = 0; j < cell->getContainerObjectsSize(); ++j) {
					ManagedReference<SceneObject*> obj = cell->getContainerObject(j);
					if (obj != nullptr) {
						debug() << "PackupStructureSession: Processing object: " << obj->getDisplayedName();
						int objCount = processObjectRecursive(obj);
						contents.itemCount += objCount;
						debug() << "PackupStructureSession: Object " << obj->getDisplayedName() << " contributed " << objCount << " to total count";
					}
				}
				info() << "PackupStructureSession: Cell " << i << " total item count so far: " << contents.itemCount << 
					", vendor count: " << contents.vendorCount;
			}
			
			if (contents.hasVendors) {
				info() << "PackupStructureSession: Building has " << contents.vendorCount << " vendors";
			}
			
			// Extract the values from our tracking structure before leaving this scope
			hasVendors = contents.hasVendors;
			itemCount = contents.itemCount;
			vendorCount = contents.vendorCount;
		}
	}

	// player is already defined at the top of the function, no need to redefine
	StringBuffer entry;
	entry << "@player_structure:confirm_packup_d1 "
		<< "@player_structure:confirm_packup_d2 \n\n"
		<< "@player_structure:confirm_packup_d3a "
		<< "\\#32CD32 @player_structure:confirm_packup_d3b \\#. "
		<< "@player_structure:confirm_packup_d4 \n\n"
		<< "@player_structure:packup_confirmation " << redeed;

	StringBuffer cond;
	cond << "@player_structure:redeed_condition \\#32CD32 "
		<< (structureObject->getMaxCondition() - structureObject->getConditionDamage()) << "/"
		<< structureObject->getMaxCondition() << "\\#.";

	StringBuffer maint;
	maint << "@player_structure:redeed_maintenance \\#"
		<< (structureObject->isRedeedable() ? "32CD32 " : "FF6347 ")
		<< structureObject->getSurplusMaintenance() << "/"
		<< structureObject->getRedeedCost() << "\\#.";
		
	// Create string for item count
	StringBuffer items;
	items << "Items to pack up: \\#32CD32 " << itemCount << "\\#.";
	
	// Create vendor status string
	StringBuffer vendorStatus;
	if (hasVendors) {
		vendorStatus << "Vendor check: \\#FF0000 " << vendorCount << " vendor" << (vendorCount == 1 ? "" : "s") << " present, MUST BE REMOVED \\#.";
	} else {
		vendorStatus << "Vendor check: \\#32CD32 No vendors present \\#.";
	}

	const ManagedReference<SuiListBox*> sui = new SuiListBox(player);
	sui->setCancelButton(true, "@no");
	sui->setOkButton(true, "@yes");
	// Disable OK button if vendors are present
	if (hasVendors) {
		sui->addSetting("3", "btnOk", "enabled", "false");
	}
	sui->setUsingObject(structureObject);
	sui->setPromptTitle(structureObject->getDisplayedName());
	
	// Add clear message about vendors if present
	if (hasVendors) {
		entry << "\n\n\\#FF0000You cannot pack up a structure that contains vendors. Please remove all vendors first.\\#";
	}
	
	sui->setPromptText(entry.toString());
	sui->addMenuItem("@player_structure:can_packup_alert " + (hasVendors ? "\\#FF6347 NO \\#" : redeed));
	sui->addMenuItem(cond.toString());
	sui->addMenuItem(maint.toString());
	sui->addMenuItem(items.toString());
	sui->addMenuItem(vendorStatus.toString());

	// Attach a small inline callback to handle Yes/No
    class PackupConfirmCallback : public SuiCallback {
    public:
        PackupConfirmCallback(ZoneServer* server, PackupStructureSession* session, bool hasVendors) : SuiCallback(server), session(session), hasVendors(hasVendors) {}
        void run(CreatureObject* creature, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) override {
            const bool cancelPressed = (eventIndex == 1);
            if (cancelPressed) {
                session->cancelSession();
                return;
            }
            
            // This is a safeguard, as the OK button should be disabled if vendors are present
            if (hasVendors) {
                creature->sendSystemMessage("You cannot pack up a structure that contains vendors. Please remove all vendors first.");
                Logger::console.info("Player attempted to pack up structure with vendors despite disabled button");
                session->cancelSession();
                return;
            }
            
            session->sendPackupCode();
        }
    private:
        ManagedReference<PackupStructureSession*> session;
        bool hasVendors;
    };

    sui->setCallback(new PackupConfirmCallback(player->getZoneServer(), _this.getReferenceUnsafeStaticCast(), hasVendors));

    player->getPlayerObject()->addSuiBox(sui);
    player->sendMessage(sui->generateMessage());

	return 0;
} // End of initializeSession function

int PackupStructureSessionImplementation::sendPackupCode() {
	if (!creatureObject->isPlayerCreature()) {
		return cancelSession();
	}

	Locker structureLock(structureObject);
	Locker creatureLock(creatureObject, structureObject);

	const auto player = creatureObject.get();
	using namespace PackupStructureConstants;
	packupCode = System::random(MAX_PACKUP_CODE - MIN_PACKUP_CODE) + MIN_PACKUP_CODE;

	const String redeed = structureObject->isRedeedable() ? "\\#32CD32 @player_structure:will_redeed_confirm \\#." : "\\#FF6347 @player_structure:will_not_redeed_confirm \\#.";

	StringBuffer entry;
	entry << "@player_structure:your_structure_prefix "
		<< redeed << " @player_structure:will_packup_suffix \n\n"
		<< "Code: " << packupCode;

    const ManagedReference<SuiInputBox*> sui = new SuiInputBox(player);
	sui->setUsingObject(structureObject);
	sui->setPromptTitle("@player_structure:confirm_packup_t");
	sui->setPromptText(entry.toString());
	sui->setCancelButton(true, "@cancel");
	sui->setMaxInputSize(6);

    class PackupCodeCallback : public SuiCallback {
    public:
        PackupCodeCallback(ZoneServer* server, PackupStructureSession* session, unsigned int code) : SuiCallback(server), session(session), code(code) {}
        void run(CreatureObject* creature, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) override {
            const bool cancelPressed = (eventIndex == 1);
            if (cancelPressed || args == nullptr || args->size() == 0) {
                session->cancelSession();
                return;
            }
            const UnicodeString& entered = args->get(0);
            uint32 enteredCode = 0;
            String enteredStr = entered.toString();
            for (int i = 0; i < enteredStr.length(); ++i) {
                char ch = enteredStr[i];
                if (ch < '0' || ch > '9') { enteredCode = 0; break; }
                enteredCode = enteredCode * 10 + static_cast<uint32>(ch - '0');
            }
            if (!session->isPackupCode(enteredCode)) {
                creature->sendSystemMessage("@player_structure:invalid_packup_code");
                session->cancelSession();
                return;
            }
            session->packupStructure();
        }
    private:
        ManagedReference<PackupStructureSession*> session;
        unsigned int code;
    };

    sui->setCallback(new PackupCodeCallback(player->getZoneServer(), _this.getReferenceUnsafeStaticCast(), packupCode));

    player->getPlayerObject()->addSuiBox(sui);
    player->sendMessage(sui->generateMessage());

	return 0;
} // End of sendPackupCode function

int PackupStructureSessionImplementation::packupStructure() {
	info() << "PackupStructureSession: packupStructure called, about to call SRStructureManager";
	
	Locker structureLock(structureObject);
	Locker creatureLock(creatureObject, structureObject);

	creatureObject->sendSystemMessage("@player_structure:processing_packup");

	if (structureObject == nullptr || structureObject->getZone() == nullptr) {
		error() << "PackupStructureSession: structureObject is null or zone is null, canceling";
		return cancelSession();
	}

	if (!structureObject->isRedeedable()) {
		info() << "PackupStructureSession: structure is not redeedable, canceling";
		creatureObject->sendSystemMessage("@player_structure:packup_items_maint");
		return cancelSession();
	}

	info() << "PackupStructureSession: calling StructureManager getSRStructureManager()->packupStructure";
	StructureManager::instance()->getSRStructureManager()->packupStructure(creatureObject);
	return 0;
} // End of packupStructure function
