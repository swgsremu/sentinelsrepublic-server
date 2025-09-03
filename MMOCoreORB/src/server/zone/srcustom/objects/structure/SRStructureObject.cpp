#include "SRStructureObject.h"

#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/objects/tangible/sign/SignObject.h"
#include "engine/util/u3d/Vector3.h"
#include "system/util/HashSet.h"

SRStructureObject::SRStructureObject()
	: controlDevice(nullptr) {
}

SRStructureObject::~SRStructureObject() {
}

SRStructureObject::SRStructureObject(const SRStructureObject& spl)
	: controlDevice(spl.controlDevice), signTemplatePath(spl.signTemplatePath) {
	// Copy packed items
	Locker locker(&lock);
	Locker otherLocker(&spl.lock);
	packedCellItems = spl.packedCellItems;
}

SRStructureObject& SRStructureObject::operator=(const SRStructureObject& list) {
	if (this == &list) {
		return *this;
	}
	
	controlDevice = list.controlDevice;
	signTemplatePath = list.signTemplatePath;
	
	// Copy packed items
	Locker locker(&lock);
	Locker otherLocker(&list.lock);
	packedCellItems = list.packedCellItems;
	
	return *this;
}

bool SRStructureObject::toBinaryStream(ObjectOutputStream* stream) {
    int _currentOffset = stream->getOffset();
    stream->writeShort(0);
    int _varCount = 0;
    
    try {
        // Save sign template path
        TypeInfo<String>::toBinaryStream(&signTemplatePath, stream);
        _varCount++;
        
        // Save packed items data if we have any
        Locker locker(&lock);
        
        if (!packedCellItems.empty()) {
            // Write the number of cells with items
            uint32 cellCount = packedCellItems.size();
            TypeInfo<uint32>::toBinaryStream(&cellCount, stream);
            _varCount++;
            
            System::out << "SRStructureObject::toBinaryStream - Saving " << cellCount << " cells with items" << endl;
            
            // For each cell, write the cell number and the items within
            for (auto& cellPair : packedCellItems) {
                // Write cell number
                TypeInfo<int>::toBinaryStream(&cellPair.first, stream);
                
                // Write number of items in this cell
                uint32 itemCount = cellPair.second.size();
                TypeInfo<uint32>::toBinaryStream(&itemCount, stream);
                
                System::out << "SRStructureObject::toBinaryStream - Cell " << cellPair.first << " has " << itemCount << " items" << endl;
                
                // Write each item's data
                for (auto& itemData : cellPair.second) {
                    TypeInfo<uint64>::toBinaryStream(&itemData.objectID, stream);
                    TypeInfo<float>::toBinaryStream(&itemData.posX, stream);
                    TypeInfo<float>::toBinaryStream(&itemData.posY, stream);
                    TypeInfo<float>::toBinaryStream(&itemData.posZ, stream);
                    TypeInfo<float>::toBinaryStream(&itemData.directionAngle, stream);
                    _varCount++;
                }
            }
        } else {
            // No packed items, just write 0 for cell count
            uint32 cellCount = 0;
            TypeInfo<uint32>::toBinaryStream(&cellCount, stream);
            _varCount++;
            
            System::out << "SRStructureObject::toBinaryStream - No packed items to save" << endl;
        }
        
    } catch (const Exception& e) {
        System::out << "SRStructureObject::toBinaryStream - Exception during serialization: " << e.getMessage() << endl;
    }
    
    // Update the variable count at the beginning of the stream
    stream->writeShort(_currentOffset, _varCount);
    
    return true;
}

bool SRStructureObject::parseFromBinaryStream(ObjectInputStream* stream) {
    // Catch any exceptions at the top level to prevent server crashes
    try {
        int _varCount = stream->readShort();
        
        // Clear packed items to ensure we don't have stale data
        Locker locker(&lock);
        packedCellItems.clear();
        
        // If no variables to read, return early
        if (_varCount <= 0) {
            return true;
        }
        
        // Read sign template path if available
        try {
            TypeInfo<String>::parseFromBinaryStream(&signTemplatePath, stream);
            _varCount--;
        } catch (const Exception& e) {
            // Silent catch for backward compatibility - older objects might not have sign data
            // Just use empty sign template path
            signTemplatePath = "";
        }
        
        // If no more variables to read, return early
        if (_varCount <= 0) {
            return true;
        }
        
        // Read packed items data if available
        try {
            uint32 cellCount = 0;
            TypeInfo<uint32>::parseFromBinaryStream(&cellCount, stream);
            _varCount--;
            
            // Sanity check the cell count
            if (cellCount > 100) {
                System::out << "SRStructureObject::parseFromBinaryStream - Unreasonable cell count: " << cellCount << ", limiting to 100" << endl;
                cellCount = 100;
            }
            
            if (cellCount > 0 && cellCount <= 100) {
                System::out << "SRStructureObject::parseFromBinaryStream - Found " << cellCount << " cells with items" << endl;
                
                // Read each cell's items
                for (uint32 i = 0; i < cellCount; i++) {
                    // Read cell number
                    int cellNum = 0;
                    try {
                        TypeInfo<int>::parseFromBinaryStream(&cellNum, stream);
                    } catch (const Exception& e) {
                        // If we can't read cell number, break out of the loop
                        break;
                    }
                    
                    // Sanity check the cell number
                    if (cellNum <= 0 || cellNum > 1000) {
                        System::out << "SRStructureObject::parseFromBinaryStream - Invalid cell number: " << cellNum << endl;
                        continue;
                    }
                    
                    // Read number of items in this cell
                    uint32 itemCount = 0;
                    try {
                        TypeInfo<uint32>::parseFromBinaryStream(&itemCount, stream);
                    } catch (const Exception& e) {
                        // If we can't read item count, break out of the loop
                        break;
                    }
                    
                    // Sanity check item count
                    if (itemCount > 1000) {
                        System::out << "SRStructureObject::parseFromBinaryStream - Too many items in cell " << cellNum << ": " << itemCount << ", limiting to 100" << endl;
                        itemCount = 100; // Cap it for safety
                    }
                    
                    std::vector<ItemPositionData> items;
                    
                    // Read each item's data
                    for (uint32 j = 0; j < itemCount; j++) {
                        ItemPositionData itemData;
                        try {
                            TypeInfo<uint64>::parseFromBinaryStream(&itemData.objectID, stream);
                            TypeInfo<float>::parseFromBinaryStream(&itemData.posX, stream);
                            TypeInfo<float>::parseFromBinaryStream(&itemData.posY, stream);
                            TypeInfo<float>::parseFromBinaryStream(&itemData.posZ, stream);
                            TypeInfo<float>::parseFromBinaryStream(&itemData.directionAngle, stream);
                            
                            // Sanity check position values and object ID
                            if (itemData.objectID > 0 && 
                                itemData.posX >= -10000 && itemData.posX <= 10000 && 
                                itemData.posY >= -10000 && itemData.posY <= 10000 && 
                                itemData.posZ >= -1000 && itemData.posZ <= 1000) {
                                items.push_back(itemData);
                            }
                        } catch (const Exception& e) {
                            // Skip this item if there's an error
                            break;
                        }
                    }
                    
                    if (!items.empty()) {
                        packedCellItems[cellNum] = std::move(items);
                    }
                }
            }
        } catch (const Exception& e) {
            // Just log at debug level since this is expected for old objects
            // System::out << "SRStructureObject::parseFromBinaryStream - Exception reading packed items (likely old format): " << e.getMessage() << endl;
        }
        
        // Only log if we actually found items
        if (!packedCellItems.empty()) {
            System::out << "SRStructureObject::parseFromBinaryStream - Successfully loaded " << packedCellItems.size() << " cells with items" << endl;
        }
        
    } catch (const Exception& e) {
        // This is a serious error that we should log
        System::out << "SRStructureObject::parseFromBinaryStream - Critical exception during parsing: " << e.getMessage() << endl;
    }
    
    return true;
}

String SRStructureObject::getPackupMessage() {
	return "";
}

void SRStructureObject::setControlDevice(ControlDevice* device) {
	Locker locker(&lock);
	controlDevice = device;
}

ControlDevice* SRStructureObject::getControlDevice() const {
	return controlDevice;
}

bool SRStructureObject::isPackedUp() const {
	return controlDevice != nullptr;
};

bool SRStructureObject::unloadFromZone(bool sendSelfDestroy) {
	return true;
}

void SRStructureObject::saveSignInfo(SignObject* sign) {
	if (sign == nullptr)
		return;
		
	Locker locker(&lock);
	signTemplatePath = sign->getObjectTemplate()->getFullTemplateString();
}

void SRStructureObject::clearPackedItems() {
	Locker locker(&lock);
	packedCellItems.clear();
	
	System::out << "SRStructureObject::clearPackedItems - Cleared packed items data" << endl;
}

void SRStructureObject::collectItems(BuildingObject* building) {
	if (building == nullptr)
		return;
		
	// Log the collection process for debugging
	System::out << "SRStructureObject::collectItems - Starting collection for building ID: " << building->getObjectID() << endl;
	
	Locker locker(&lock);
	packedCellItems.clear();
	int total = building->getTotalCellNumber();
	System::out << "SRStructureObject::collectItems - Total cells: " << total << endl;
	
	for (int i = 1; i <= total; ++i) {
		CellObject* cell = building->getCell(i);
		if (cell == nullptr) {
			System::out << "SRStructureObject::collectItems - Cell " << i << " is null" << endl;
			continue;
		}
		
		std::vector<ItemPositionData> items;
		int size = cell->getContainerObjectsSize();
		System::out << "SRStructureObject::collectItems - Cell " << i << " has " << size << " objects" << endl;
		
		for (int j = 0; j < size; ++j) {
			auto obj = cell->getContainerObject(j);
			// Skip creature objects and structure terminal objects (these will be recreated)
			if (obj != nullptr && !obj->isCreatureObject() && 
				!(obj->isTerminal() && obj->getObjectTemplate()->getFullTemplateString().indexOf("terminal/structure") != -1) &&
				!obj->isSignObject()) { // Also skip sign objects
				
				// Store the object ID along with its position and orientation
				ItemPositionData itemData;
				itemData.objectID = obj->getObjectID();
				itemData.posX = obj->getPositionX();
				itemData.posY = obj->getPositionY();
				itemData.posZ = obj->getPositionZ();
				
				// Store direction as a simple angle
				const Quaternion* dirPtr = obj->getDirection();
				if (dirPtr != nullptr) {
					itemData.directionAngle = dirPtr->getRadians() * (180.0f / M_PI);
				} else {
					itemData.directionAngle = 0.0f;
				}
				
				items.push_back(itemData);
				
				System::out << "SRStructureObject::collectItems - Adding object: " << obj->getObjectID() 
					<< " Template: " << obj->getObjectTemplate()->getFullTemplateString()
					<< " Position: [" << itemData.posX << ", " << itemData.posY << ", " << itemData.posZ << "]" << endl;
			} else if (obj != nullptr) {
				System::out << "SRStructureObject::collectItems - Skipping object: " << obj->getObjectID() 
					<< " Type: " << (obj->isCreatureObject() ? "Creature" : 
									(obj->isTerminal() ? "Terminal" : 
									(obj->isSignObject() ? "Sign" : "Other"))) << endl;
			}
		}
		if (!items.empty()) {
			packedCellItems.emplace(i, std::move(items));
			System::out << "SRStructureObject::collectItems - Added " << packedCellItems[i].size() << " items for cell " << i << endl;
		}
	}
	
	System::out << "SRStructureObject::collectItems - Finished collecting items, total cells with items: " 
		<< packedCellItems.size() << endl;
}

void SRStructureObject::restoreItems(BuildingObject* building, ZoneServer* zoneServer) {
	if (building == nullptr || zoneServer == nullptr) {
		System::out << "SRStructureObject::restoreItems - Building or ZoneServer is null" << endl;
		return;
	}
	
	System::out << "SRStructureObject::restoreItems - Starting restoration for building ID: " << building->getObjectID() << endl;
	System::out << "SRStructureObject::restoreItems - Total cells with packed items: " << packedCellItems.size() << endl;
	
	// Debug: Log each cell and item count to verify data was properly loaded
	for (auto& pair : packedCellItems) {
		System::out << "SRStructureObject::restoreItems - Cell " << pair.first << " has " 
		            << pair.second.size() << " packed items to restore" << endl;
	}
	
	Locker locker(&lock);
	int itemsRestored = 0;
	int itemsSkipped = 0;
	int itemsFailed = 0;
	int totalItems = 0;
	
	// Count total items for progress tracking
	for (auto& pair : packedCellItems) {
		totalItems += pair.second.size();
	}
	
	System::out << "SRStructureObject::restoreItems - Total items to restore: " << totalItems << endl;
	int progressMilestone = std::max(1, totalItems / 10); // Report progress at 10% intervals
	int lastMilestone = 0;
	
	for (auto& pair : packedCellItems) {
		int cellNum = pair.first;
		CellObject* cell = building->getCell(cellNum);
		
		if (cell == nullptr) {
			System::out << "SRStructureObject::restoreItems - Cell " << cellNum << " is null" << endl;
			itemsFailed += pair.second.size();
			continue;
		}
		
		System::out << "SRStructureObject::restoreItems - Processing " << pair.second.size() << " items for cell " << cellNum << endl;
		
		// Collect existing structure terminals in this cell for deduplication
		HashSet<String> existingTerminalTemplates;
		int currentCellSize = cell->getContainerObjectsSize();
		
		for (int i = 0; i < currentCellSize; ++i) {
			auto existingObj = cell->getContainerObject(i);
			if (existingObj != nullptr && existingObj->isTerminal() && 
				existingObj->getObjectTemplate()->getFullTemplateString().indexOf("terminal/structure") != -1) {
				existingTerminalTemplates.add(existingObj->getObjectTemplate()->getFullTemplateString());
				System::out << "SRStructureObject::restoreItems - Found existing terminal: " << existingObj->getObjectTemplate()->getFullTemplateString() << endl;
			}
		}
		
		for (const auto& itemData : pair.second) {
			auto obj = zoneServer->getObject(itemData.objectID);
			
			if (obj == nullptr) {
				System::out << "SRStructureObject::restoreItems - Object ID " << itemData.objectID << " not found in ZoneServer" << endl;
				itemsFailed++;
				continue;
			}
			
			// Skip restoring structure terminals that already exist
			if (obj->isTerminal() && 
				obj->getObjectTemplate()->getFullTemplateString().indexOf("terminal/structure") != -1) {
				String terminalTemplate = obj->getObjectTemplate()->getFullTemplateString();
				if (existingTerminalTemplates.contains(terminalTemplate)) {
					System::out << "SRStructureObject::restoreItems - Skipping duplicate terminal: " << terminalTemplate << endl;
					itemsSkipped++;
					continue; // Skip this terminal as we already have one of this type
				}
				existingTerminalTemplates.add(terminalTemplate); // Remember this terminal type
			}
			
			// Skip sign objects to avoid duplicate signs
			if (obj->isSignObject()) {
				System::out << "SRStructureObject::restoreItems - Skipping sign object: " << itemData.objectID << endl;
				itemsSkipped++;
				continue;
			}
			
			try {
				Locker objLocker(obj);
				
				if (obj->getZone() == nullptr) {
					// Set the position and orientation before transferring the object
					obj->initializePosition(itemData.posX, itemData.posZ, itemData.posY);
					
					// Convert direction angle back to radians and set as heading
					float radians = itemData.directionAngle * (M_PI / 180.0f);
					obj->setDirection(radians);
					
					// Reinsert into the cell only if it no longer resides in a zone
					System::out << "SRStructureObject::restoreItems - Restoring object ID: " << itemData.objectID 
						<< " Template: " << obj->getObjectTemplate()->getFullTemplateString() 
						<< " Position: [" << itemData.posX << ", " << itemData.posY << ", " << itemData.posZ << "]" << endl;
					
					bool success = cell->transferObject(obj, -1, true);
					
					if (success) {
						// After transfer, verify the object's position
						if (obj->getPositionX() != itemData.posX || 
							obj->getPositionY() != itemData.posY || 
							obj->getPositionZ() != itemData.posZ) {
							
							// Some objects might reset their positions after transfer, so set it again
							obj->initializePosition(itemData.posX, itemData.posZ, itemData.posY);
							
							// Convert direction angle back to radians and set as heading
							float radians = itemData.directionAngle * (M_PI / 180.0f);
							obj->setDirection(radians);
							obj->updateToDatabase();
							
							System::out << "SRStructureObject::restoreItems - Re-adjusted position for object ID: " 
								<< itemData.objectID << endl;
						}
						
						itemsRestored++;
					} else {
						System::out << "SRStructureObject::restoreItems - Failed to transfer object ID: " << itemData.objectID 
							<< " Template: " << obj->getObjectTemplate()->getFullTemplateString() 
							<< " - Possible collision or permission issue" << endl;
						
						// Check if object is in a container already
						ManagedReference<SceneObject*> parent = obj->getParent().get();
						if (parent != nullptr) {
							System::out << "SRStructureObject::restoreItems - Object already has a parent: " 
								<< parent->getObjectID() << endl;
						}
						
						itemsFailed++;
					}
					
					// Report progress at milestones
					int currentProgress = itemsRestored + itemsSkipped + itemsFailed;
					if (currentProgress - lastMilestone >= progressMilestone) {
						int percentComplete = (currentProgress * 100) / totalItems;
						System::out << "SRStructureObject::restoreItems - Progress: " << percentComplete 
							<< "% complete (" << currentProgress << "/" << totalItems << " items processed)" << endl;
						lastMilestone = currentProgress;
					}
				} else {
					System::out << "SRStructureObject::restoreItems - Object ID " << itemData.objectID << " already has a zone, skipping" << endl;
					itemsSkipped++;
				}
			} catch (Exception& e) {
				System::out << "SRStructureObject::restoreItems - Exception restoring object ID: " << itemData.objectID 
					<< " Error: " << e.getMessage() << endl;
				
				// Log details about the exception for debugging
				System::out << "SRStructureObject::restoreItems - Exception details: " << e.getMessage() << endl;
				
				itemsFailed++;
			}
		}
	}
	
	// Calculate success rate
	float successRate = (totalItems > 0) ? ((float)itemsRestored / totalItems * 100.0f) : 100.0f;
	
	System::out << "SRStructureObject::restoreItems - Restoration complete: " 
		<< itemsRestored << " items restored (" << successRate << "% success rate), " 
		<< itemsSkipped << " items skipped, " 
		<< itemsFailed << " items failed" << endl;
	
	// Log memory stats to help diagnose memory issues
	System::out << "SRStructureObject::restoreItems - Clearing memory for packedCellItems with " 
		<< packedCellItems.size() << " cells" << endl;
		
	// The broadcasting to players is handled in StructureControlDeviceImplementation.cpp
	// No need to broadcast here as it can cause crashes with null parameters
}

void to_json(nlohmann::json& j, const SRStructureObject& p) {
}