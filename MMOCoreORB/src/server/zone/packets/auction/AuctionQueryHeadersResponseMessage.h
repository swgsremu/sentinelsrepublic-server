/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

// FUTURE CLEANUP: Search for "REMOVE HYBRID FALLBACK" to remove backward compatibility code
// after all legacy vendor items without cost-per-unit in names have expired (added 2025-08-18)

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

	    	UnicodeString name = il->getItemName();
	    	
	    	// TODO: REMOVE HYBRID FALLBACK - After all legacy items expire (added 2025-08-18)
	    	// This fallback can be removed once all pre-existing vendor items without "/cpu" in names have expired.
	    	// When removed, this method can simply be: insertUnicode(name);
	    	// For backward compatibility: if item name doesn't already contain cost per unit, add it
	    	if (name.toString().indexOf("/cpu") == -1) {
	    		try {
	    			ManagedReference<SceneObject*> obj = player->getZoneServer()->getObject(il->getAuctionedItemObjectID());
	    			
	    			if (obj != nullptr && obj->isTangibleObject()) {
	    				TangibleObject* tangible = cast<TangibleObject*>(obj.get());
	    				
	    				if (tangible != nullptr) {
	    					int useCount = tangible->getUseCount();
	    					int actualCount = (useCount > 0) ? useCount : 1;  // Protect against zero and negative values
	    					int price = il->getPrice();
	    					
	    					if (price > 0) {
	    						float costPerUnit = (float)price / (float)actualCount;
	    						char cpuBuffer[32];
	    						snprintf(cpuBuffer, sizeof(cpuBuffer), " - %.2f/cpu", costPerUnit);
	    						String suffix = String(cpuBuffer);
	    						// Protect against overly long names (arbitrary limit of 200 chars total)
	    						if ((name.toString().length() + suffix.length()) <= 200) {
	    							name = name + suffix;
	    						}
	    					}
	    				}
	    			}
	    		} catch (Exception& e) {
	    			// If there's any error calculating cost per unit, just use the original name
	    			// Log at debug level to avoid spam, only if debugging is enabled
	    			#ifdef DEBUG_AUCTION_SEARCH
	    			player->error("Exception calculating cost per unit for item " + String::valueOf(il->getAuctionedItemObjectID()) + ": " + e.getMessage());
	    			#endif
	    		}
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
