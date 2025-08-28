/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

// FUTURE CLEANUP: Search for "REMOVE HYBRID FALLBACK" to remove backward compatibility code
// after all legacy vendor items without cost-per-unit in names have expired (added 2025-08-18)
// Updated 2025-08-28: Changed format from "/cpu" to "CPU", restricted to resource containers only,
// and added logic to strip CPU info from non-resource items for consistency

#ifndef AUCTIONQUERYHEADERSRESPONSEMESSAGE_H_
#define AUCTIONQUERYHEADERSRESPONSEMESSAGE_H_

#include "engine/service/proto/BaseMessage.h"
#include "server/zone/objects/auction/AuctionItem.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/tangible/TangibleObject.h"
#include "server/zone/ZoneServer.h"
#include "system/lang/Math.h"

class AuctionQueryHeadersResponseMessage : public BaseMessage {

	Vector<ManagedReference<AuctionItem*> > itemList;

	SortedVector<String> locationList;

	ManagedReference<CreatureObject*> player;

public:
	AuctionQueryHeadersResponseMessage(int screen, int counter, CreatureObject* player) : BaseMessage() {
		insertShort(0x08);
		insertInt(0xFA500E52);  // opcode

		insertInt(counter);
		insertInt(screen); // Vendor screen number

		setCompression(true);

		this->player = player;

		locationList.setNoDuplicateInsertPlan();
	}

	void addItemToList(AuctionItem* ai) {
		locationList.put(ai->getVendorUID());
		locationList.put(ai->getOwnerName());

		if(ai->isAuction() && ai->getStatus() == AuctionItem::FORSALE)
			locationList.put("");
		else
			locationList.put(ai->getBidderName());

		itemList.add(ai);
	}

	void dumpLocationList() {
		int llSize = locationList.size();

		insertInt(llSize);

		for (int i = 0; i < locationList.size(); i++) {
			insertAscii(locationList.get(i));
		}
	}

	void dumpItemNameList() {
		int ilSize = itemList.size();

		insertInt(ilSize);

		for (int i = 0; i < itemList.size(); i++) {
			AuctionItem* il = itemList.get(i);

	    	UnicodeString name = il->getItemName(); // Start with the original name
	    	
	    	try {
	    		// Get the object to determine its type
	    		ManagedReference<SceneObject*> obj = player->getZoneServer()->getObject(il->getAuctionedItemObjectID());
	    		
	    		if (obj != nullptr) {
	    			// Check if this is a resource container
	    			bool isResource = (obj->getGameObjectType() & SceneObjectType::RESOURCECONTAINER);
	    			
	    			// For brute-force string handling, convert to std::string for more control
	    			std::string stdName = name.toString().toCharArray();
	    			std::string cleanName = stdName;
	    			
	    			// EXTREME VERSION: Use regex-like manual parsing to find and strip CPU information
	    			// Pattern: look for " - <numbers>.<numbers>/cpu" or " - <numbers>.<numbers> CPU"
	    			
	    			// First pass - find the pattern " - X.XX/cpu"
	    			size_t cpuPos = stdName.find("/cpu");
	    			if (cpuPos != std::string::npos) {
	    				// Look backwards for a dash
	    				size_t dashPos = stdName.rfind(" - ", cpuPos);
	    				if (dashPos != std::string::npos) {
	    					// Make sure there are numbers between dash and /cpu
	    					bool hasNumbers = false;
	    					for (size_t i = dashPos + 3; i < cpuPos; ++i) {
	    						if (isdigit(stdName[i]) || stdName[i] == '.') {
	    							hasNumbers = true;
	    							break;
	    						}
	    					}
	    					
	    					if (hasNumbers) {
	    						// Cut off everything from dash to the end
	    						cleanName = stdName.substr(0, dashPos);
	    					}
	    				}
	    			}
	    			
	    			// Second pass - find the pattern " - X.XX CPU"
	    			if (cleanName == stdName) { // Only check if we didn't already modify it
	    				cpuPos = stdName.find(" CPU");
	    				if (cpuPos != std::string::npos) {
	    					size_t dashPos = stdName.rfind(" - ", cpuPos);
	    					if (dashPos != std::string::npos) {
	    						// Make sure there are numbers between dash and CPU
	    						bool hasNumbers = false;
	    						for (size_t i = dashPos + 3; i < cpuPos; ++i) {
	    							if (isdigit(stdName[i]) || stdName[i] == '.') {
	    								hasNumbers = true;
	    								break;
	    							}
	    						}
	    						
	    						if (hasNumbers) {
	    							// Cut off everything from dash to the end
	    							cleanName = stdName.substr(0, dashPos);
	    						}
	    					}
	    				}
	    			}
	    			
	    			// At this point, cleanName has the base name without any CPU info
	    			// Set name to this clean version
	    			name = UnicodeString(cleanName.c_str());
	    			
	    			// For resource containers only, add back the CPU information
	    			if (isResource && obj->isTangibleObject()) {
	    				TangibleObject* tangible = cast<TangibleObject*>(obj.get());
	    				
	    				if (tangible != nullptr) {
	    					int useCount = tangible->getUseCount();
	    					int actualCount = (useCount > 0) ? useCount : 1;
	    					int price = il->getPrice();
	    					
	    					if (price > 0) {
	    						float costPerUnit = (float)price / (float)actualCount;
	    						char cpuBuffer[32];
	    						snprintf(cpuBuffer, sizeof(cpuBuffer), " - %.2f CPU", costPerUnit);
	    						std::string suffix = cpuBuffer;
	    						
	    						// Add the CPU info
	    						if (cleanName.length() + suffix.length() <= 200) {
	    							name = UnicodeString((cleanName + suffix).c_str());
	    						}
	    					}
	    				}
	    			}
	    			// For non-resources, name is already the clean version
	    		}
	    	} catch (Exception& e) {
	    		// If there's any error calculating cost per unit or processing the name, just use the original name
	    		// Log at debug level to avoid spam, only if debugging is enabled
	    		#ifdef DEBUG_AUCTION_SEARCH
	    		player->error("Exception processing item name for item " + String::valueOf(il->getAuctionedItemObjectID()) + ": " + e.getMessage());
	    		#endif
	    	}
	    	// END TODO: REMOVE HYBRID FALLBACK
	    	
	    	insertUnicode(name); //name
		}
	}

	void dumpItemInfoList() {
		Time expireTime;
		uint64 currentTime = expireTime.getMiliTime() / 1000;

		int ilSize = itemList.size();

		insertInt(ilSize);

		for (int i = 0; i < itemList.size(); i++) {
			AuctionItem* il = itemList.get(i);

			int accessFee = 0;
			ManagedReference<SceneObject*> vendor = player->getZoneServer()->getObject(il->getVendorID());
			if(vendor != nullptr) {
				ManagedReference<SceneObject*> parent = vendor->getRootParent();
				if(parent != nullptr && parent->isBuildingObject()) {
					BuildingObject* building = cast<BuildingObject*>(parent.get());
					if(building != nullptr)
						accessFee = building->getAccessFee();
				}
			}

			insertLong(il->getAuctionedItemObjectID()); //item id
			insertByte(i);  // List item String number

			insertInt(il->getPrice()); //item cost.

			uint32 expire = il->getExpireTime() > currentTime ? il->getExpireTime() - currentTime : 0;

			insertInt(expire);

	    	if (il->isAuction())
	    		insertByte(0);
	    	else
	    		insertByte(1);

	    	insertShort(locationList.find(il->getVendorUID()));

	    	insertLong(il->getOwnerID()); // seller ID

	    	insertShort(locationList.find(il->getOwnerName()));

	    	if(il->isAuction() && il->getStatus() == AuctionItem::FORSALE) {
	    		insertLong(0);
	    		insertShort(locationList.find(""));
	    	} else {
	    		insertLong(il->getBuyerID()); // buyer ID
	    		insertShort(locationList.find(il->getBidderName()));
	    	}

	    	insertInt(il->getProxy()); // my proxy not implemented yet
	    	insertInt(il->getPrice()); // my bid default to price

	    	insertInt(il->getItemType());

	    	//insertInt(il->getAuctionOptions()); // autionOptions 0x400 = Premium, 0x800 = withdraw
	    	int additionalValues = 0;

	    	if (il->getOwnerID() == player->getObjectID() &&
	    			(il->getStatus() == AuctionItem::FORSALE || il->getStatus() == AuctionItem::OFFERED)) {
	    		additionalValues |= 0x800;
	    	}

	    	insertInt(il->getAuctionOptions() | additionalValues);
	    	//insertInt(10);

	    	insertInt(accessFee);
		}
	}

	void createMessage(int offset = 0, bool continues = false) {
		dumpLocationList();

		dumpItemNameList();

		dumpItemInfoList();

		insertShort(offset); // Item list start offset

		insertByte(continues); // more to come?
	}

	inline int getListSize() {
		return itemList.size();
	}

};

#endif /*AUCTIONQUERYHEADERSRESPONSEMESSAGE_H_*/
