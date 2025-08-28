/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef LOOTCOMMAND_H_
#define LOOTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/group/GroupLootTask.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/zone/objects/scene/variables/ContainerPermissions.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/TreeEntry.h"
#include "server/zone/Zone.h"

class LootCommand : public QueueCommand {

public:
	enum {
		NOPICKUPITEMS = 0,
		ITEMFOROTHER = 1,
		PICKEDANDREMAINING = 2,
		PICKEDANDEMPTY = 3
	};
	
	// Constants
	static constexpr float LOOT_RANGE = 16.0f;

	LootCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}
public:
	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
	        if (!checkStateMask(creature))
	            return INVALIDSTATE;

	        if (!checkInvalidLocomotions(creature))
	            return INVALIDLOCOMOTION;

	        ZoneServer* zoneServer = server->getZoneServer();
	        if (zoneServer == nullptr)
	            return GENERALERROR;

	        bool lootArea = arguments.toString().beginsWith("area");
	        bool lootAll = arguments.toString().beginsWith("all");
	        PlayerManager* playerManager = zoneServer->getPlayerManager();
	        
	        if (playerManager == nullptr)
	            return GENERALERROR;

	        if (!lootArea) {
	            ManagedReference<SceneObject*> targetObject = zoneServer->getObject(target);

	            if (targetObject == nullptr || !targetObject->isAiAgent())
	                return INVALIDTARGET;

	            AiAgent* agent = targetObject.castTo<AiAgent*>();

	            if (agent == nullptr)
	                return INVALIDTARGET;

	            Locker locker(agent, creature);
	            
	            return attemptLoot(creature, agent, playerManager, lootAll);

	        } else {	            
	            
	            ManagedReference<SceneObject*> inventory = creature->getSlottedObject("inventory");
	            if (inventory->isContainerFullRecursive()) {
	                creature->sendSystemMessage("@error_message:inv_full"); //Your inventory is full.
	                return GENERALERROR;
	            }

	            Zone* zone = creature->getZone();
	            if (zone == nullptr)
	                return GENERALERROR;

	            int lootsFound = 0;
	            int lootsProcessed = 0;
	            SortedVector<ManagedReference<AiAgent*> > lootees;
	            lootees.setNoDuplicateInsertPlan();

	            // Handle corpses in buildings/dungeons/cells
	            ManagedReference<SceneObject*> parent = creature->getParent().get();
	            if (parent != nullptr && parent->isCellObject()) {
	                ManagedReference<CellObject*> cell = dynamic_cast<CellObject*>(parent.get());
	                if (cell != nullptr) {
	                    SceneObject* parentObj = cell->getParent().get();
	                    ManagedReference<BuildingObject*> building = parentObj != nullptr ? dynamic_cast<BuildingObject*>(parentObj) : nullptr;
	                    if (building != nullptr) {
	                        for (int i = 1; i <= building->getTotalCellNumber(); i++) {  // Start at 1, not 0
	                            ManagedReference<CellObject*> buildingCell = building->getCell(i);
	                            if (buildingCell != nullptr) {
	                                for (int j = 0; j < buildingCell->getContainerObjectsSize(); j++) {
	                                    ManagedReference<SceneObject*> content = buildingCell->getContainerObject(j);
	                                    if (content != nullptr && content->isAiAgent()) {
	                                        ManagedReference<AiAgent*> agent = dynamic_cast<AiAgent*>(content.get());
	                                        if (agent != nullptr && agent->isDead() && !lootees.contains(agent) && agent->isInRange(creature, LOOT_RANGE)) {
	                                            lootees.put(agent);
	                                            lootsFound++;
	                                        }
	                                    }
	                                }
	                            }
	                        }
	                    }
	                }
	            } else {
	                // Handle corpses in outdoor areas
	                SortedVector<TreeEntry*> objects(512, 512);
	                zone->getInRangeObjects(creature->getPositionX(), creature->getPositionZ(), creature->getPositionY(), LOOT_RANGE, &objects, true, true);

	                for (int i = 0; i < objects.size(); i++) {
	                    ManagedReference<SceneObject*> object = dynamic_cast<SceneObject*>(objects.get(i));
	                    
	                    if (object == nullptr || !object->isAiAgent())
	                        continue;

	                    ManagedReference<AiAgent*> agent = dynamic_cast<AiAgent*>(object.get());
	                    
	                    if (agent == nullptr || !agent->isDead() || lootees.contains(agent))
	                        continue;

	                    if (agent->isInRange(creature, LOOT_RANGE)) {
	                        lootees.put(agent);
	                        lootsFound++;
	                    }
	                }
	            }
	            
	            // Check if any loot is found
	            if (lootsFound == 0) {
	                creature->sendSystemMessage("There is nothing to loot in this area.");
	                return GENERALERROR;
	            }
	            
	            // Limit the number of corpses to 25 to prevent abuse/lag
	            if (lootsFound > 25) {
	                creature->sendSystemMessage("A maximum of 25 corpses can be looted at once.");
	                lootsFound = 25;
	            }

	            for (int i = 0; i < lootees.size() && i < 25; ++i) {
	                ManagedReference<AiAgent*> agent = lootees.get(i);
	                
	                if (agent == nullptr)
	                    continue;
	                
	                Locker locker(agent, creature);
	                
	                if (attemptLoot(creature, agent, playerManager, true) == SUCCESS) {
	                    lootsProcessed++;
	                }
	                
	                locker.release();
	            }
	            
	            // Send summary message
	            if (lootsProcessed > 0) {
	                creature->sendSystemMessage("Looted " + String::valueOf(lootsProcessed) + " corpse" + (lootsProcessed == 1 ? "." : "s."));
	            } else {
	                creature->sendSystemMessage("Failed to loot any corpses. Your inventory might be full.");
	            }
	            
	            return SUCCESS;
	        }
	        return SUCCESS;
	    }

	/**
	 * Helper method to attempt looting a single corpse
	 * @param creature The player attempting to loot
	 * @param agent The corpse being looted
	 * @param playerManager The player manager instance
	 * @param lootAll Whether to loot all items or just open the container
	 * @return SUCCESS if looting was handled, or appropriate error code
	 */
	int attemptLoot(CreatureObject* creature, AiAgent* agent, PlayerManager* playerManager, bool lootAll) const {
	    if (agent == nullptr)
	        return INVALIDTARGET;

	    if (!agent->isDead() || creature->isDead())
	        return GENERALERROR;

	    if (!checkDistance(agent, creature, LOOT_RANGE)) {
	        creature->sendSystemMessage("@error_message:target_out_of_range"); //"Your target is out of range for this action."
	        return GENERALERROR;
	    }

	    // Get the corpse's inventory
	    SceneObject* lootContainer = agent->getSlottedObject("inventory");
	    if (lootContainer == nullptr)
	        return GENERALERROR;

	    if (playerManager == nullptr)
	        return GENERALERROR;

	    const ContainerPermissions* permissions = lootContainer->getContainerPermissions();
	    if (permissions == nullptr)
	        return GENERALERROR;

	    // Determine the loot rights
	    uint64 ownerID = permissions->getOwnerID();
	    bool looterIsOwner = (ownerID == creature->getObjectID());
	    bool groupIsOwner = (ownerID == creature->getGroupID());

	    // Allow player to loot the corpse if they own it
	    if (looterIsOwner) {
	        if (lootAll) {
	            playerManager->lootAll(creature, agent);
	        } else {
	            // Check if the corpse's inventory contains any items
	            if (lootContainer->getContainerObjectsSize() < 1) {
	                creature->sendSystemMessage("@error_message:corpse_empty"); //"You find nothing else of value on the selected corpse."
	                playerManager->rescheduleCorpseDestruction(creature, agent);
	            } else {
	                agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
	                lootContainer->openContainerTo(creature);
	            }
	        }
	        return SUCCESS;
	    }

	    // If player and their group don't own the corpse, pick up any owned items left on corpse due to full inventory, then fail
	    if (!groupIsOwner) {
	        int pickupResult = pickupOwnedItems(agent, creature, lootContainer);
	        if (pickupResult < 2) { // Player didn't pickup an item nor is one available for them
	            StringIdChatParameter noPermission("error_message", "no_corpse_permission"); //"You do not have permission to access this corpse."
	            creature->sendSystemMessage(noPermission);
	            return GENERALERROR;
	        } else if (pickupResult == PICKEDANDEMPTY) {
	            playerManager->rescheduleCorpseDestruction(creature, agent);
	            return SUCCESS;
	        }
	        return SUCCESS;
	    }

	    // If looter's group is the owner, attempt to pick up any owned items, then process group loot rule
	    int pickupResult = pickupOwnedItems(agent, creature, lootContainer);
	    switch (pickupResult) {
	    case NOPICKUPITEMS: // No items available for anyone to pickup
	        break;
	    case ITEMFOROTHER: // No items available for looter to pickup, but one is available for someone else
	        agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
	        lootContainer->openContainerTo(creature);
	        return SUCCESS;
	    case PICKEDANDREMAINING: // An item was available for the looter, there are items remaining
	        return SUCCESS;
	    case PICKEDANDEMPTY: // An item was available for the looter, there are NO items remaining
	        playerManager->rescheduleCorpseDestruction(creature, agent);
	        return SUCCESS;
	    default:
	        break;
	    }

	    ManagedReference<GroupObject*> group = creature->getGroup();
	    if (group == nullptr)
	        return GENERALERROR;

	    try {
	        GroupLootTask* task = new GroupLootTask(group, creature, agent, lootAll);
	        if (task != nullptr) {
	            task->execute();
	        }
	    } catch (Exception& e) {
	        // Safely handle any exceptions during task creation/execution
	        creature->error("Exception in loot task: " + e.getMessage());
	    }

	    return SUCCESS;
	}

	int pickupOwnedItems(AiAgent* ai, CreatureObject* creature, SceneObject* lootContainer) const {
		/* Return codes:
		 * NOPICKUPITEMS: No items available for anyone to pickup.
		 * ITEMFOROTHER: No items available for looter to pickup, but one is available for someone else.
		 * PICKEDANDREMAINING: An item was available for the looter, there are items remaining.
		 * PICKEDANDEMPTY: An item was available for the looter, there are NO items remaining.
		 */

		bool attemptedPickup = false;
		bool pickupAvailableOther = false;

		int totalItems = lootContainer->getContainerObjectsSize();
		if (totalItems < 1) return NOPICKUPITEMS;

		ContainerPermissions* contPerms = lootContainer->getContainerPermissionsForUpdate();
		if (contPerms == nullptr) {
			return NOPICKUPITEMS;
		}

		SceneObject* playerInventory = creature->getSlottedObject("inventory");
		if (playerInventory == nullptr) {
			return NOPICKUPITEMS;
		}

		// Check each loot item to see if the player owns it.
		for (int i = totalItems - 1; i >= 0; --i) {
			SceneObject* object = lootContainer->getContainerObject(i);
			if (object == nullptr) continue;

			ContainerPermissions* itemPerms = object->getContainerPermissionsForUpdate();
			if (itemPerms == nullptr) continue;

			//Check if player owns the loot item.
			uint64 itemOwnerID = itemPerms->getOwnerID();
			if (itemOwnerID == creature->getObjectID()) {

				// Attempt to transfer the item to the player.
				attemptedPickup = true;
				if (playerInventory->isContainerFullRecursive()) {
					StringIdChatParameter full("group", "you_are_full"); //"Your Inventory is full."
					creature->sendSystemMessage(full);
					return PICKEDANDREMAINING;
				}

				uint64 originalOwner = contPerms->getOwnerID();
				contPerms->setOwner(creature->getObjectID());
				TransactionLog trx(ai, creature, object, TrxCode::NPCLOOTCLAIM);

				if (creature->getZoneServer()->getObjectController()->transferObject(object, playerInventory, -1, true)) {
					itemPerms->clearDenyPermission("player", ContainerPermissions::OPEN);
					itemPerms->clearDenyPermission("player", ContainerPermissions::MOVECONTAINER);
					trx.commit();
				} else {
					trx.abort() << "Failed to transferObject to player";
				}

				contPerms->setOwner(originalOwner);

			} else if (itemOwnerID != 0)
				pickupAvailableOther = true;
		}

		//Determine which result code to return.
		if (attemptedPickup) {
			if (lootContainer->getContainerObjectsSize() > 0)
				return PICKEDANDREMAINING;
			else
				return PICKEDANDEMPTY;
		}

		if (pickupAvailableOther)
			return ITEMFOROTHER;

		return NOPICKUPITEMS;
	}
};
#endif //LOOTCOMMAND_H_