/*
 *	FishingManagerImplementation.cpp
 *
 *	Created on: Apr 11, 2010
 *	Refactor on: Aug. 3, 2021 - By: Hakry
 *	Author: swgemu
 */

#include "server/zone/managers/minigames/FishingManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/tangible/fishing/FishingPoleObject.h"
#include "server/zone/objects/tangible/fishing/FishingBaitObject.h"
#include "server/zone/objects/tangible/fishing/FishObject.h"
#include "server/zone/objects/resource/ResourceContainer.h"
#include "terrain/manager/TerrainManager.h"
#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/zone/managers/minigames/events/FishingEvent.h"
#include "server/zone/managers/minigames/events/FishingSplashEvent.h"
#include "server/zone/managers/minigames/FishingSession.h"
#include "server/zone/managers/loot/LootManager.h"
#include "system/util/VectorMap.h"

// #define DEBUG_FISHING

void FishingManagerImplementation::initialize() {
	info("Loading configuration.");

	if (!loadConfigData() && fishingEnabled) {
		info("Failed to load configuration lua.");
	}
}

bool FishingManagerImplementation::loadConfigData() {
	Lua* lua = new Lua();
	lua->init();

	if (!lua->runFile("scripts/managers/fishing_manager.lua")) {
		delete lua;
		return false;
	}

	fishingEnabled = lua->getGlobalBoolean("fishingEnabled");

	if (!fishingEnabled) {
		info("Fishing is disabled.");
		return false;
	}

	LuaObject fishTypes = lua->getGlobalObject("fishTypes");

	if (fishTypes.isValidTable()) {
		for (int i = 1; i <= fishTypes.getTableSize(); ++i) {
			String fishName = fishTypes.getStringAt(i);

			fishType.add(fishName);
		}
	}
	fishTypes.pop();

	info("Loaded a total of " + String::valueOf(fishType.size()) + " Fish Species.");

	LuaObject colors = lua->getGlobalObject("fishColors");

	if (colors.isValidTable()) {
		for (int i = 1; i <= colors.getTableSize(); ++i) {
			LuaObject colorsByPlanet = colors.getObjectAt(i);

			if (colorsByPlanet.isValidTable()) {
				String planet = colorsByPlanet.getStringAt(1);
				int colorInt = colorsByPlanet.getIntAt(2);

				fishColors.put(planet, colorInt);
			}
			colorsByPlanet.pop();
		}
	}
	colors.pop();

	info("Loaded Fish Colors for " + String::valueOf(fishColors.size()) + " Planets.");

	LuaObject lengths = lua->getGlobalObject("fishLengths");

	if (lengths.isValidTable()) {
		for (int i = 1; i <= lengths.getTableSize(); ++i) {
			LuaObject lengthsByFish = lengths.getObjectAt(i);

			if (lengthsByFish.isValidTable()) {
				String fishSpecies = lengthsByFish.getStringAt(1);
				float length = lengthsByFish.getFloatAt(2);

				fishLengths.put(fishSpecies, length);
			}
			lengthsByFish.pop();
		}
	}
	lengths.pop();

	info("Loaded " + String::valueOf(fishLengths.size()) + " fish lengths.");

	LuaObject actions = lua->getGlobalObject("fishingActions");

	if (actions.isValidTable()) {
		for (int i = 1; i <= actions.getTableSize(); ++i) {
			String action = actions.getStringAt(i);

			fishingActions.add(action);
		}
	}
	actions.pop();

	info("Loaded " + String::valueOf(fishingActions.size()) + " Fishing Actions.");

	LuaObject states = lua->getGlobalObject("fishingStates");

	if (states.isValidTable()) {
		for (int i = 1; i <= states.getTableSize(); ++i) {
			String state = states.getStringAt(i);

			fishingStates.add(state);
		}
	}
	states.pop();

	info("Loaded " + String::valueOf(fishingStates.size()) + " Fishing States.");

	LuaObject baitStat = lua->getGlobalObject("baitStatus");

	if (baitStat.isValidTable()) {
		for (int i = 1; i <= baitStat.getTableSize(); ++i) {
			String status = baitStat.getStringAt(i);

			baitStatus.add(status);
		}
	}
	baitStat.pop();

	info("Loaded " + String::valueOf(baitStatus.size()) + " Bait Status Types.");

	delete lua;
	lua = nullptr;

	return true;
}

int FishingManagerImplementation::startFishing(CreatureObject* player) {
	if (!fishingEnabled || player == nullptr) {
		return NOFISHING;
	}

	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole == nullptr) {
		player->sendSystemMessage("@fishing:must_have_pole");
		return NOFISHING;
	}

	ManagedReference<FishingBaitObject*> bait = getBait(player);

	if (bait == nullptr) {
		player->sendSystemMessage("@fishing:bait_your_pole");
		return NOFISHING;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return NOFISHING;
	}

	if (player->isSwimming()) {
		player->sendSystemMessage("@fishing:bad_cast");
		return NOFISHING;
	}

	Vector3 location;
	int locationCheck = checkLocation(player, pole->getQuality(), location);

	switch(locationCheck) {
		case NOFISHING:
			return locationCheck; // Player was nullptr
		case BADCAST:
			player->sendSystemMessage("@fishing:too_far"); // Your cast goes astray and lands at a non-fishable spot.
			return locationCheck;
		case NOWATER:
			player->sendSystemMessage("@fishing:invalid_location");
			return locationCheck;
		default:
			break;
	}

	ManagedReference<SceneObject*> markerObject = createMarker(location, zone);

	if (markerObject == nullptr) {
		return NOFISHING;
	}

	String moodString = player->getMoodString();

	createFishingSession(player, createFishingEvent(player, WAITING), markerObject, DONOTHING, 0, 0, WAITING, moodString);

	Reference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr) {
		return NOFISHING;
	}

	uint32 boxID = createWindow(player, 0);

	fishingSession->setFishBoxID(boxID);

	player->doAnimation("fishing_cast");
	player->setMoodString("fishing");
	player->sendSystemMessage("@fishing:start_fishing");

	player->registerObserver(ObserverEventType::POSITIONCHANGED, _this.getReferenceUnsafeStaticCast());
	player->registerObserver(ObserverEventType::PLAYERKILLED, _this.getReferenceUnsafeStaticCast());
	player->registerObserver(ObserverEventType::STARTCOMBAT, _this.getReferenceUnsafeStaticCast());

	createSplash(location.getX(), location.getY(), location.getZ(), zone, player);

	return STARTFISHING;
}

int FishingManagerImplementation::checkLocation(CreatureObject* player, int quality, Vector3& location) {
	if (player == nullptr) {
		return NOFISHING;
	}

	auto zone = player->getZone();

	if (zone == nullptr || !zone->isGroundZone()) {
		return NOFISHING;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		return NOFISHING;
	}

	auto terrainManager = planetManager->getTerrainManager();

	if (terrainManager == nullptr) {
		return NOFISHING;
	}

	float angle = player->getDirectionAngle();

	if (angle > 360) {
		angle = angle - 360;
	}

	float randVal = (float)System::random(2);
	float distance = Math::min((Math::max(fabs(1.0 - (quality / 10.f)), 2.1) + randVal), 10.0); // Calculates the Distance, using the Pole's Quality

	angle = 2 * M_PI * angle / 360;

	location = player->getWorldPosition();

	float x = location.getX() + sin(angle) * (distance);
	float y = location.getY() + cos(angle) * (distance);
	float z = location.getZ();

	location.setX(x);
	location.setY(y);

	if (!terrainManager->getWaterHeight(x, y, z)) {
		return BADCAST;
	}

	location.setZ(z);

	if (z < zone->getHeight(x, y)) {
		return NOWATER;
	}

	return STARTFISHING;
}

void FishingManagerImplementation::createFishingSession(CreatureObject* player, FishingEvent* event, SceneObject* marker, int nextAction, int fish, uint32 boxID, int fishingState, String& mood) {
	if (player == nullptr || event == nullptr || marker == nullptr) {
		return;
	}

	// Fishing Session is created and dropped in stopFishing or notifyCloseContainer
	player->addActiveSession(SessionFacadeType::FISHING, new FishingSession(event, marker, nextAction, fish, boxID, fishingState, mood));
}

void FishingManagerImplementation::stopFishing(CreatureObject* player, uint32 boxID, bool destroyMarker) {
	if (player == nullptr) {
		return;
	}

#ifdef DEBUG_FISHING
	player->info(true) << "FishingManagerImplementation::stopFishing -- called for " << player->getDisplayedName();
#endif

	// Remove the observers from the player
	player->dropObserver(ObserverEventType::POSITIONCHANGED, _this.getReferenceUnsafeStaticCast());
	player->dropObserver(ObserverEventType::PLAYERKILLED, _this.getReferenceUnsafeStaticCast());
	player->dropObserver(ObserverEventType::STARTCOMBAT, _this.getReferenceUnsafeStaticCast());

	// Stop the fishing Event
	stopFishingEvent(player);

	// Get the session
	Reference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();
	ManagedReference<SceneObject*> marker = nullptr;

	String moodString = "none";

	if (fishingSession != nullptr) {
		// Get the store mood string
		moodString = fishingSession->getMoodString();

		// Clear the session fishing state
		fishingSession->setFishingState(NOTFISHING);

		fishingSession->setEvent(nullptr);

		marker = fishingSession->getMarker();
	}

	// Clear the fishing mood off of the player
	player->setMoodString(moodString);

	player->sendSystemMessage("@fishing:stop_fishing");

	if (!player->isInCombat() && !player->isIncapacitated() && !player->isDead()) {
		player->doAnimation("fishing_reel");
	}

	uint32 id = boxID;

	if (boxID == 0) {
		id = getFishBoxID(player);
	}

	// We can now drop the session off the player
	player->dropActiveSession(SessionFacadeType::FISHING);

	// Fishing Window
	closeMenu(player, id);

	// The only instance when the marker is not automatically destroyed is when there is loot in it the player must retrieve
	if (destroyMarker && marker != nullptr) {
		Locker locker(marker, player);
		removeMarker(player, marker);
	}
}

// Player is pre-locked upon function call
void FishingManagerImplementation::continueFishing(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

#ifdef DEBUG_FISHING
	player->info(true) << "FishingManagerImplementation::continueFishing -- called for " << player->getDisplayedName();
#endif

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr) {
		return;
	}

	uint32 boxID = fishingSession->getFishBoxID();

	// Player needs a pole to continue fishing
	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole == nullptr) {
		stopFishing(player, boxID, true);
		return;
	}

	// Make sure the player is alive and notincapacitated or in combat
	if (player->isInCombat() || player->isDead() || player->isIncapacitated()) {
		stopFishing(player, boxID, true);
		return;
	}

	// Get the fishing marker
	ManagedReference<SceneObject*> marker = fishingSession->getMarker();

	int nextAction = fishingSession->getNextAction();
	int fish = fishingSession->getFish();
	int state = fishingSession->getFishingState();

	String moodString = fishingSession->getMoodString();

	// Close Current Window
	if (state != REELGAME) {
		closeMenu(player, boxID);
	}

	// Random Event & Mishap
	int chance = System::random(30);
	int vegStat = vegetation(marker);
	int fishDensity = density(marker);
	int poleMod = 0;

	if (pole->getQuality() != 0) {
		poleMod = (int)ceil((float)pole->getQuality() / 10);
	}

	// Accounting for bad Bait
	ManagedReference<FishingBaitObject*> bait = getBait(player);

	if (bait != nullptr) {
		int baitState = bait->getFreshness();

		if (state < CATCH && baitState > SOGGY && System::random(15) < baitState) {
			mishapEvent("@fishing:toss_bait", player, marker, boxID, true, moodString);
			return; // NO BAIT
		}
	}

#ifdef DEBUG_FISHING
	player->info(true) << "State: " << state << " Chance: " << chance << " Mishap: " << String::valueOf(MISHAP) << " Next Action: " << nextAction << " Pole Mod: " << poleMod << " Vegetation: " << vegStat;
#endif

	// Play Fishing Animation
	animate(player, nextAction);

	switch (state) {
		case WAITING: {
			int newFish = getFish(player, marker);

			if (chance + poleMod <= MISHAP + vegStat) { // Pole decreases chance of Snag
				fishingProceed(player, nextAction, marker, newFish, boxID, SNAGGED, false, moodString);
			} else {
				if (chance - poleMod <= fishDensity * 1.5) { // Nibble
					if (nextAction >= TUGUP && nextAction <= REEL) {
						ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

						if (newMarker != nullptr) {
							fishingProceed(player, nextAction, newMarker, newFish, boxID, NIBBLE, true, moodString);
						}
					} else {
						fishingProceed(player, nextAction, marker, newFish, boxID, NIBBLE, true, moodString);
					}
				} else { // Waiting
					if (nextAction >= TUGUP && nextAction <= REEL) {
						ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

						if (newMarker != nullptr) {
							fishingProceed(player, nextAction, newMarker, newFish, boxID, WAITING, false, moodString);
						}
					} else {
						fishingProceed(player, nextAction, marker, newFish, boxID, WAITING, false, moodString);
					}
				}
			}
			break;
		}
		case SNAGGED: {
			if (nextAction >= TUGUP && nextAction <= REEL && chance + poleMod - (vegStat / 2) >= PROCEED) { // Pole Mod increase chance to not snag and vegetation increase chance to snag
				fishingProceed(player, nextAction, marker, fish, boxID, WAITING, false, moodString);
			} else {
				fishingProceed(player, nextAction, marker, fish, boxID, SNAGGED, false, moodString);
			}
			break;
		}
		case NIBBLE: {
			if (chance + poleMod <= MISHAP) { // Pole decreases chance of mishap
				mishapEvent("@fishing:line_snap", player, marker, boxID, true, moodString);
				break;
			} else if ((chance + poleMod >= (PROCEED + 10 - (fishDensity * 2))) && (nextAction >= TUGUP && nextAction <= REEL)) { // BITE
				ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

				if (newMarker != nullptr) {
					fishingProceed(player, nextAction, newMarker, fish, boxID, BITE, true, moodString);
				} else {
					fishingProceed(player, nextAction, marker, fish, boxID, BITE, true, moodString);
				}
				break;
			} else if (nextAction >= TUGUP && nextAction <= REEL) { // Action entered & BITE calculation failed
				ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

				if (newMarker != nullptr) {
					fishingProceed(player, nextAction, newMarker, fish, boxID, NIBBLE, true, moodString);
				} else {
					fishingProceed(player, nextAction, newMarker, fish, boxID, WAITING, true, moodString);
				}
			} else {
				if (chance <= 10) {
					fishingProceed(player, nextAction, marker, fish, boxID, WAITING, true, moodString);
				} else {
					fishingProceed(player, nextAction, marker, fish, boxID, NIBBLE, true, moodString);
				}
			}
			break;
		}
		case BITE: {
			if (chance + poleMod <= MISHAP) { // Pole decreases chance of MISHAP
				if (chance < 3) { // Lost Bait
					mishapEvent("@fishing:lost_bait", player, marker, boxID, true, moodString);
					break;
				} else { // Fish pulled out the line, but the player has no penalty
					mishapEvent("@fishing:line_spooled", player, marker, boxID, false, moodString);
					break;
				}
			} else if (chance + poleMod >= PROCEED && nextAction >= TUGUP && nextAction <= REEL) {
				ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

				if (newMarker != nullptr) {
					fishingProceed(player, nextAction, newMarker, fish, boxID, CATCH, true, moodString);
				} else {
					fishingProceed(player, nextAction, marker, fish, boxID, BITE, true, moodString);
				}
			} else {
				fishingProceed(player, nextAction, marker, fish, boxID, NIBBLE, false, moodString);
			}
			break;
		}
		case CATCH: {
			if (chance + poleMod <= MISHAP) {
				mishapEvent("@fishing:lost_catch", player, marker, boxID, false, moodString);
				break;
			} else if (nextAction == DONOTHING || nextAction == REEL) {
				fishingProceed(player, nextAction, marker, fish, boxID, REELING, true, moodString);
			} else {
				fishingProceed(player, nextAction, marker, fish, boxID, CATCH, true, moodString);
			}
			break;
		}
		case REELING: {
			ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, true);

			if (newMarker != nullptr)
				fishingProceed(player, nextAction, newMarker, fish, boxID, REELING, true, moodString);
			break;
		}
		case REELGAME: {
			if (chance + poleMod <= MISHAP - 5) { // Pole decreases chance of mishap
					mishapEvent("@fishing:tore_bait", player, marker, boxID, true, moodString);
			} else {
				if (player->isInRange(marker, 2.0)) {
					success(player, fish, marker, boxID);
				} else {
					ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, true);

					if (newMarker != nullptr) {
						fishingProceed(player, nextAction, newMarker, fish, boxID, REELGAME, true, moodString);
					} else {
						fishingProceed(player, nextAction, newMarker, fish, boxID, REELGAME, true, moodString);
					}
				}
			}
			break;
		}
	}
}

void FishingManagerImplementation::success(CreatureObject* player, int fish, SceneObject* marker, uint32 boxID) {
	if (player == nullptr) {
		return;
	}

#ifdef DEBUG_FISHING
	player->info(true) << "FishingManagerImplementation::success -- called for " << player->getDisplayedName();
#endif

	auto zone = player->getZone();

	if (zone == nullptr) {
		return;
	}

	auto zoneServer = player->getZoneServer();

	if (zoneServer == nullptr) {
		return;
	}

	ManagedReference<LootManager*> lootManager = zoneServer->getLootManager();

	if (lootManager == nullptr) {
		return;
	}

	TransactionLog trx(TrxCode::FISHING, player);

	if (fish > 80) { // Non-fish Item Caught
		int chance = System::random(200) + player->getSkillMod("luck");
		uint64 lootID = 0;

		if (chance > 190) { // Rare Loot
			if (System::random(4) > 1) {
				lootID = lootManager->createLoot(trx, marker, "weapons_all", 40);
			} else {
				lootID = lootManager->createLoot(trx, marker, "armor_all", 40);
			}
		} else { // Junk Loot Drop
			lootID = lootManager->createLoot(trx, marker, "junk", 40);
		}

		if (lootID > 0) {
			trx.commit();
			sendReward(player, marker, nullptr);
		} else {
			trx.abort() << "FishingManagerImplementation -- Loot Object ID == 0";
		}
	} else {
		// Chance to toss fish away
		int badLuck = System::random(20);

		if (badLuck <= 1) {
			player->sendSystemMessage("@fishing:size_matters");
			stopFishing(player, boxID, true);

			trx.abort() << "@fishing:size_matters";
			return;
		} else {
			String fishName = fishType.get(fish);
			String lootFish = "object/tangible/fishing/fish/" + fishName + ".iff";
			ManagedReference<FishObject*> lootFishObject = zoneServer->createObject(lootFish.hashCode(), 2).castTo<FishObject*>();

			if (lootFishObject != nullptr) {
				Locker lootLocker(lootFishObject, player);

				String time = getTime();
				String name = player->getFirstName() + " " + player->getLastName();

				// Determine fish length
				float length = fishLengths.get(fishName);
				float totalLength = length;
				float harvestingMod = player->getSkillMod("creature_harvesting") + 5.f;
				float randBonus = System::frandom(length * 0.5f) - System::frandom(length * 0.5f);

				totalLength += randBonus;

				// Bonus for creature harvesting mod
				if (harvestingMod > 0) {
					harvestingMod = (System::frandom(harvestingMod) / 100.0f) + 1.f;
					totalLength *= harvestingMod;
				}

				String zoneName = zone->getZoneName();

				lootFishObject->setAttributes(name, zoneName, time, (totalLength / 100.f));

				int xp = (totalLength / length) * 90;

				ManagedReference<PlayerManager*> playerManager = zoneServer->getPlayerManager();

				if (playerManager != nullptr) {
					playerManager->awardExperience(player, "camp", xp, true);
				}

				int color = 1;

				for (int j = 1; j < fishColors.size(); ++j) {
					String fishingPlanet = fishColors.elementAt(j).getKey();

					if (fishingPlanet == zoneName) {
						color = fishColors.elementAt(j).getValue();
					}
				}

				lootFishObject->setCustomizationVariable("/private/index_color_1", color);

				String baitString = "object/tangible/fishing/bait/bait_chum.iff";
				ManagedReference<TangibleObject*> baitObject = zoneServer->createObject(baitString.hashCode(), 2).castTo<TangibleObject*>();

				if (baitObject != nullptr) {
					Locker baitLocker(baitObject, player);

					TransactionLog trxBait(TrxCode::FISHING, player);

					int useCount = System::random(5) + 1;

					if (useCount >= 1)
						baitObject->setUseCount(useCount, true);

					if (lootFishObject->transferObject(baitObject, -1, true)) {
						trxBait.setSubject(baitObject);
						trxBait.addState("subjectObjectID", baitObject->getObjectID());
						trxBait.addState("subjectUseCount", useCount);
						trxBait.groupWith(trx);

						lootFishObject->broadcastObject(baitObject, true);
					} else {
						baitObject->destroyObjectFromDatabase(true);
					}
				}

				String resourceString = "seafood_fish_" + zoneName;
				int amount = harvestingMod * 50;

				ManagedReference<ResourceManager*> resourceManager = zoneServer->getResourceManager();
				ManagedReference<SceneObject*> resource = cast<SceneObject*>(resourceManager->harvestResource(player, resourceString, amount));

				if (resource != nullptr) {
					Locker resourceLocker(resource, player);

					if (lootFishObject->transferObject(resource, -1, true)) {
						TransactionLog trxResource(TrxCode::FISHING, player);
						trxResource.setSubject(resource);
						trxResource.addState("subjectResourceType", resourceString);
						trxResource.addState("subjectResourceQuantity", amount);
						trxResource.groupWith(trx);

						lootFishObject->broadcastObject(resource, true);
					} else {
						resource->destroyObjectFromDatabase(true);
					}
				}

				sendReward(player, marker, cast<SceneObject*>(lootFishObject.get()));

				trx.setSubject(lootFishObject);
				trx.addState("subjectFishType", fishName);
				trx.addState("subjectFishLength", length);
				trx.addState("experienceAwarded", xp);
				trx.commit();
			}
		}
	}

	stopFishing(player, boxID, false);
}

void FishingManagerImplementation::sendReward(CreatureObject* player, SceneObject* marker, SceneObject* loot) {
	if (player != nullptr && marker != nullptr) {
		Locker markerLocker(marker);

		ContainerPermissions* permissions = marker->getContainerPermissionsForUpdate();
		permissions->setOwner(player->getObjectID());

		if (loot != nullptr) {
			if (marker->transferObject(loot, -1, true)) {
				String itemName = "";
				loot->getObjectName()->getFullPath(itemName);

				StringIdChatParameter body("fishing", "prose_notify_catch");
				body.setTT(itemName);
				player->sendSystemMessage(body);
			} else {
				loot->destroyObjectFromDatabase(true);
			}
		} else {
			player->sendSystemMessage("@fishing:reel_loot");
		}

		marker->openContainerTo(player);
	}
}

uint32 FishingManagerImplementation::createWindow(CreatureObject* player, uint32 boxID) {
	if (player == nullptr)
		return -1;

	ManagedReference<SceneObject*> marker = getFishMarker(player);

	if (marker == nullptr)
		return -1;

	// clear old one
	closeMenu(player, boxID);

	int veg = vegetation(marker);
	int den = density(marker);

	ManagedReference<FishingBaitObject*> bait = getBait(player);

	if (bait == nullptr)
		return -1;

	int freshness = bait->getFreshness();

	if (freshness >= MUSH) {
		freshness = 2;
	} else if (freshness >= SOGGY) {
		freshness = 1;
	} else {
		freshness = 0;
	}

	int nextAct = getNextAction(player);

	if (nextAct >= fishingActions.size())
		nextAct = 0;

	String prompt = "STATUS : " + fishingStates.get(getFishingState(player) % 7)
		+ "\nBAIT STATUS : " + baitStatus.get(freshness % 3)
		+ "\nLINE RANGE : " + String::valueOf(ceil(player->getDistanceTo(marker) * 100) / 100)
		+ "\nFISH DENSITY : " + getPropertyString(den % 6)
		+ "\nVEGETATION : " + getPropertyString(veg % 6)
		+ "\nNEXT ACTION : " + fishingActions.get(nextAct);

	// create new window
	ManagedReference<SuiListBox*> box = new SuiListBox(player, SuiWindowType::FISHING, 0);

	box->setPromptTitle("@fishing:sui_title");
	box->setPromptText(prompt);

	box->addMenuItem("@fishing:fa_tug_up", 0);
	box->addMenuItem("@fishing:fa_tug_right", 1);
	box->addMenuItem("@fishing:fa_tug_left", 2);
	box->addMenuItem("@fishing:fa_small_reel", 3);
	box->addMenuItem("@fishing:fa_stop", 4);

	player->getPlayerObject()->addSuiBox(box);
	player->sendMessage(box->generateMessage());

	return box->getBoxID();
}

void FishingManagerImplementation::closeMenu(CreatureObject* player, uint32 boxID) {
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost != nullptr) {
		ghost->removeSuiBoxType(SuiWindowType::FISHING);
	}
}

int FishingManagerImplementation::vegetation(SceneObject* marker) {
	if (marker == nullptr)
		return 0;

	return (int)ceil(fabs(marker->getPositionX() / 100)) % 6;
}

int FishingManagerImplementation::density(SceneObject* marker) {
	if (marker == nullptr)
		return 0;

	return (int)ceil(fabs(marker->getPositionY() / 100)) % 6;
}

int FishingManagerImplementation::getFish(CreatureObject* player, SceneObject* marker) {
	int chance = System::random(99);

#ifdef DEBUG_FISHING
	StringBuffer msg;
	msg << " Starting getFish chance = " << chance << "  ";
#endif

	if (player == nullptr || marker == nullptr) {
		return chance;
	}

	int vegStat = vegetation(marker);

	if (vegStat > 0 && chance < 85) {
		chance += vegStat * 5;
	}

#ifdef DEBUG_FISHING
	msg << " chance after vegeation bonus = " << chance << "  ";
#endif

	if (chance < 85) {
		ManagedReference<FishingPoleObject*> pole = getPole(player);
		if (pole == nullptr)
			return chance;

		ManagedReference<FishingBaitObject*> bait = getBait(player);
		if (bait == nullptr)
			return chance;

		chance = (int)((pole->getQuality() * 0.06)			// ACCOUNT FOR POLE QUALITY - quality is 50 at init
					   + (vegStat * -0.75)		// Vegetation Decrease chance to catch a fish
					   + (density(player) * 0.75)			// ACCOUNT FOR FISH DENSITY - is 0 if density method fails
					   + ((3 - bait->getFreshness()) * 3.0) // ACCOUNT FOR BAIT STATUS - freshness is 0 at init
					   + player->getSkillMod("luck") // ACCOUNT FOR LUCK
					   + (System::random(20))) // RANDOM BIAS
				 % 7;						   // MUX NUMBER TO FISH 0-6
	}

#ifdef DEBUG_FISHING
	msg << " Final Chance: " << chance;
	player->info(true) << msg.toString();
#endif

	return chance;
}

int FishingManagerImplementation::getNextAction(CreatureObject* player) {
	if (player == nullptr)
		return DONOTHING;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr)
		return DONOTHING;

	return fishingSession->getNextAction();
}

void FishingManagerImplementation::setNextAction(CreatureObject* player, int next) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr)
		return;

	if ((next >= DONOTHING) && (next <= STOPFISHING)) {
		fishingSession->setNextAction(next);
	}
}

FishingPoleObject* FishingManagerImplementation::getPole(CreatureObject* player) {
	if (player == nullptr) {
		return nullptr;
	}

	SceneObject* poleSceneO = player->getSlottedObject("hold_r");

	if (poleSceneO == nullptr || !poleSceneO->isFishingPoleObject()) {
		return nullptr;
	}

	ManagedReference<FishingPoleObject*> fishingPole = cast<FishingPoleObject*>(poleSceneO);

	if (fishingPole == nullptr) {
		return nullptr;
	}

	return fishingPole;
}

FishingBaitObject* FishingManagerImplementation::getBait(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	Locker locker(_this.getReferenceUnsafeStaticCast());

	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole != nullptr) {
		if (pole->isFishingPoleObject() && pole->isContainerFull()) {
			ManagedReference<FishingBaitObject*> bait = pole->getContainerObject(0).castTo<FishingBaitObject*>();

			if (bait != nullptr) {
				if (bait->isFishingBait()) {
					return bait;
				}
			}
		}
	}

	return nullptr;
}

uint32 FishingManagerImplementation::getFishBoxID(CreatureObject* player) {
	if (player == nullptr)
		return -1;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession != nullptr) {
		return fishingSession->getFishBoxID();
	}

	return -1;
}

void FishingManagerImplementation::setFishBoxID(CreatureObject* player, uint32 boxID) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession != nullptr) {
		fishingSession->setFishBoxID(boxID);
	}
}

int FishingManagerImplementation::getFishingState(CreatureObject* player) {
	if (player == nullptr)
		return NOTFISHING;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr) {
		return NOTFISHING;
	}

	return fishingSession->getFishingState();
}

void FishingManagerImplementation::setFishingState(CreatureObject* player, int state) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr)
		return;

	if ((state >= NOTFISHING) && (state <= REELGAME)) {
		fishingSession->setFishingState(state);
	}
}

SceneObject* FishingManagerImplementation::getFishMarker(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession != nullptr)
		return fishingSession->getMarker();
	else
		return player;
}

void FishingManagerImplementation::setFishMarker(CreatureObject* player, SceneObject* marker) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr)
		return;

	fishingSession->setMarker(marker);
}

void FishingManagerImplementation::fishingProceed(CreatureObject* player, int nextAction, SceneObject* marker, int fish, uint32 boxID, int newstate, bool sendPlayerMessage, String& moodString) {
	if (player == nullptr || marker == nullptr) {
		return;
	}

	int oldstate = getFishingState(player);

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (sendPlayerMessage) {
		switch (newstate) {
		case WAITING:
			if (oldstate == SNAGGED)
				player->sendSystemMessage("@fishing:line_free");
			break;
		case SNAGGED: {
			player->sendSystemMessage("@fishing:fs_snag");
			break;
		}
		case NIBBLE: {
			if (oldstate == SNAGGED) {
				player->sendSystemMessage("@fishing:line_free");
			} else {
				player->sendSystemMessage("@fishing:fs_nibble");
			}

			marker->showFlyText("fishing", "fly_nibble", 50, 200, 50);
			break;
		}
		case BITE: {
			marker->showFlyText("fishing", "fly_bite", 230, 150, 30);
			player->sendSystemMessage("@fishing:fs_bite");

			createSplash(marker->getPositionX(), marker->getPositionY(), marker->getPositionZ(), player->getZone(), player);
			break;
		}
		case CATCH: {
			marker->showFlyText("fishing", "fly_catch", 240, 240, 65);
			player->sendSystemMessage("@fishing:fs_caught");

			createSplash(marker->getPositionX(), marker->getPositionY(), marker->getPositionZ(), player->getZone(), player);
			break;
		}
		case REELING: {
			if (fishingSession) {
				player->sendSystemMessage("@fishing:reel_in");
				FishingEvent* fishingEvent = createFishingEvent(player, REELGAME);

				if (fishingEvent != nullptr) {
					fishingSession->setEvent(fishingEvent);
					fishingSession->update(REEL, marker, fish, 0, REELGAME);
				}
			}
			break;
		}
		case REELGAME: {
			if (fishingSession) {
				FishingEvent* fishingEvent = createFishingEvent(player, newstate);

				if (fishingEvent != nullptr) {
					fishingSession->setEvent(fishingEvent);
					fishingSession->update(REEL, marker, fish, 0, newstate);
				}
			}
			break;
		}
		default:
			break;
		}
	}

	if (nextAction >= DONOTHING && nextAction < REELING) {
		ManagedReference<FishingBaitObject*> bait = getBait(player);

		if (bait != nullptr && System::random(100) < 5) {
			bait->lessFresh();
		}

		if (player->isInRange(marker, 2.f)) {
			stopFishing(player, boxID, true);
		}
	}

	if ((newstate != REELING) && (newstate != REELGAME)) {
		if (fishingSession) {
			FishingEvent* fishingEvent = createFishingEvent(player, newstate);

			if (fishingEvent != nullptr) {
				fishingSession->update(DONOTHING, marker, fish, boxID, newstate);
				fishingSession->setFishBoxID(createWindow(player, boxID));
				fishingSession->setEvent(fishingEvent);
			}
		}
	}
}

void FishingManagerImplementation::mishapEvent(const String& text, CreatureObject* player, SceneObject* marker, uint32 boxID, bool losebait, String& moodString) {
	if ((player != nullptr) && (!text.isEmpty())) {
		player->sendSystemMessage(text);

		if (losebait) {
			stopFishing(player, boxID, true);
			loseBait(player);
		} else {
			ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

			if (fishingSession) {
				FishingEvent* fishingEvent = createFishingEvent(player, WAITING);

				if (fishingEvent != nullptr) {
					fishingSession->update(DONOTHING, getFishMarker(player), getFish(player, marker), boxID, WAITING);
					fishingSession->setFishBoxID(createWindow(player, boxID));
					fishingSession->setEvent(fishingEvent);
				}
			}
		}
	}
}

bool FishingManagerImplementation::loseBait(CreatureObject* player) {
	if (player == nullptr)
		return false;

	ManagedReference<SceneObject*> pole = player->getSlottedObject("hold_r");

	if (pole != nullptr) {
		if (pole->isFishingPoleObject() && pole->isContainerFull()) {
			if (pole->getContainerObject(0) != nullptr) {
				ManagedReference<SceneObject*> bait = pole->getContainerObject(0);

				if (!bait->isFishingBait())
					return false;

				ManagedReference<FishingBaitObject*> fishBait = cast<FishingBaitObject*>(bait.get());

				Locker fishBaitLocker(fishBait);

				if (fishBait->getUseCount() > 1) {
					fishBait->decreaseUseCount();
					fishBait->setFreshness(FRESH);
				} else {
					bait->destroyObjectFromWorld(true);

					if (bait->isPersistent())
						bait->destroyObjectFromDatabase(true);
				}

				return true;
			}
		}
	}

	return false;
}

void FishingManagerImplementation::animate(CreatureObject* player, int nextAction) {
	if (player == nullptr)
		return;

	switch (nextAction) {
	case TUGUP:
		player->doAnimation("fishing_tug_back");
		break;
	case TUGRIGHT:
		player->doAnimation("fishing_tug_right");
		break;
	case TUGLEFT:
		player->doAnimation("fishing_tug_left");
		break;
	case REEL:
		player->doAnimation("fishing_reel");
		break;
	}
}

SceneObject* FishingManagerImplementation::createMarker(Vector3& location, Zone* zone) {
	if (zone == nullptr) {
		return nullptr;
	}

	String marker = "object/tangible/fishing/marker.iff";
	ManagedReference<SceneObject*> markerObject = zone->getZoneServer()->createObject(marker.hashCode(), 0);

	if (markerObject == nullptr) {
		return nullptr;
	}

	Locker locker(markerObject);

	markerObject->initializePosition(location.getX(), location.getZ(), location.getY());

	if (!zone->transferObject(markerObject, -1, true)) {
		return nullptr;
	}

	markerObject->registerObserver(ObserverEventType::CLOSECONTAINER, _this.getReferenceUnsafeStaticCast());

	return markerObject;
}

void FishingManagerImplementation::createSplash(float x, float y, float z, Zone* zone, CreatureObject* player) {
	if (player == nullptr || zone == nullptr)
		return;

	auto zoneServer = player->getZoneServer();

	if (zoneServer == nullptr)
		return;

	String splash = "object/tangible/fishing/splash.iff";
	ManagedReference<SceneObject*> splashObject = zoneServer->createObject(splash.hashCode(), 0);

	if (splashObject == nullptr)
		return;

	Locker locker(splashObject);

	splashObject->initializePosition(x, z + 0.5, y);
	zone->transferObject(splashObject, -1, true);

	createFishingSplashEvent(splashObject);
}

bool FishingManagerImplementation::checkUpdateMarker(CreatureObject* player, Vector3& location) {
	bool beached = false;

	float newZ = location.getZ();

	if (!player->getZone()->getPlanetManager()->getTerrainManager()->getWaterHeight(location.getX(), location.getY(), newZ)) {
		beached = true;
	}

	newZ = player->getZone()->getHeight(location.getX(), location.getY());

	if (location.getZ() < newZ) {
		location.setZ(newZ);
		beached = true;
	}

	return beached;
}

SceneObject* FishingManagerImplementation::updateMarker(CreatureObject* player, SceneObject* marker, uint32 boxID, bool reelGame) {
	if (player == nullptr || marker == nullptr) {
		return nullptr;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return nullptr;
	}

	Locker markerLocker(marker);

	// Calculate new bobber position
	Vector3 newMarkerLoc;
	Vector3 markerLoc;
	float playerX = player->getPositionX();
	float playerY = player->getPositionY();

	markerLoc.setX(marker->getPositionX());
	markerLoc.setY(marker->getPositionY());

	newMarkerLoc.setX(playerX + ((markerLoc.getX() - playerX) * 0.9));
	newMarkerLoc.setY(playerY + ((markerLoc.getY() - playerY) * 0.9));
	newMarkerLoc.setZ(marker->getPositionZ());

	if (reelGame) {
		int event = System::random(12);
		switch (event) {
			case 1:
				// Fish Charge
				newMarkerLoc.setX(playerX + ((markerLoc.getX() - playerX) * 0.7));
				newMarkerLoc.setY(playerY + ((markerLoc.getY() - playerY) * 0.7));

				player->sendSystemMessage("@fishing:fish_charge");
				break;
			case 2:
				// Fight Easy
				player->sendSystemMessage("@fishing:fish_fight_easy");
				break;
			case 3:
				// Fight Hard
				newMarkerLoc.setX(markerLoc.getX() + System::random(2));
				newMarkerLoc.setY(markerLoc.getY() + System::random(2));

				player->sendSystemMessage("@fishing:fish_fight_hard");
				break;
			case 4:
				// Fight Away
				newMarkerLoc.setX(markerLoc.getX() + System::random(2));
				newMarkerLoc.setY(markerLoc.getY() + System::random(2));

				player->sendSystemMessage("@fishing:fish_fight_hard");
				break;
			case 5: {
				// Fish Run
				float angle = player->getDirectionAngle();

				if (angle > 360) {
					angle = angle - 360;
				}

				angle = 2 * M_PI * angle / 360;

				newMarkerLoc.setX(markerLoc.getX() + sin(angle) * System::random(5));
				newMarkerLoc.setY(markerLoc.getY() + cos(angle) * System::random(5));

				player->sendSystemMessage("@fishing:fish_run");
				break;
			}
			default:
				// succeed in drawing your catch closer
				player->sendSystemMessage("@fishing:fish_fight_closer");
				break;
		}
	}

	if (checkUpdateMarker(player, newMarkerLoc)) { // Beached
		if (reelGame) {
			player->sendSystemMessage("@fishing:loot_beached");
			newMarkerLoc = markerLoc;
		} else {
			player->sendSystemMessage("@fishing:loc_unfishable");
			stopFishing(player, boxID, true);
			return nullptr;
		}
	}

	removeMarker(player, marker);

	ManagedReference<SceneObject*> newMarker = createMarker(newMarkerLoc, zone);

	if (newMarker == nullptr)
		return nullptr;

	Locker lock(player, marker);

	setFishMarker(player, newMarker);

	return newMarker;
}

// Marker and player are prelocked
void FishingManagerImplementation::removeMarker(CreatureObject* player, SceneObject* marker) {
	if (player == nullptr || marker == nullptr || marker->isPlayerCreature())
		return;

	// Clear fish marker for player fishing session
	setFishMarker(player, nullptr);

	for (int i = marker->getContainerObjectsSize() - 1; i >= 0; --i) {
		ManagedReference<SceneObject*> object = marker->getContainerObject(i);

		if (object == nullptr)
			continue;

		Locker clock(object, player);

		if (object->isPersistent()) {
			object->destroyObjectFromDatabase(true);
		}

		object->destroyObjectFromWorld(true);
	}

	// Destroy the marker
	marker->destroyObjectFromWorld(true);

	if (marker->isPersistent()) {
		marker->destroyObjectFromDatabase(true);
	}
}

void FishingManagerImplementation::removeSplash(SceneObject* splash) {
	if (splash == nullptr)
		return;

	splash->destroyObjectFromWorld(true);

	if (splash->isPersistent()) {
		splash->destroyObjectFromDatabase(true);
	}
}

void FishingManagerImplementation::createFishingSplashEvent(SceneObject* splash) {
	if (splash == nullptr)
		return;

	Reference<FishingSplashEvent*> splashEvent = new FishingSplashEvent(splash);

	if (splashEvent != nullptr)
		splashEvent->schedule(1000);
}

FishingEvent* FishingManagerImplementation::createFishingEvent(CreatureObject* player, int state) {
	if (player == nullptr)
		return nullptr;

	player->removePendingTask("fishing");

	Reference<FishingEvent*> fishingEvent = new FishingEvent(player, state);

	player->addPendingTask("fishing", fishingEvent, 7000);

	return fishingEvent;
}

void FishingManagerImplementation::stopFishingEvent(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	player->removePendingTask("fishing");

	FishingEvent* fishingEvent = getFishingEvent(player);

	if (fishingEvent == nullptr) {
		return;
	}

	fishingEvent->cancel();

	fishingEvent = nullptr;
}

FishingEvent* FishingManagerImplementation::getFishingEvent(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr)
		return nullptr;

	FishingEvent* fishingEvent = fishingSession->getEvent();

	return fishingEvent;
}

bool FishingManagerImplementation::hasFishingSession(CreatureObject* player) {
	return ((player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>()) != nullptr);
}

int FishingManagerImplementation::notifyCloseContainer(CreatureObject* player, SceneObject* container) {
	if (player == nullptr || container == nullptr)
		return 1;

	player->dropActiveSession(SessionFacadeType::FISHING);

	Reference<FishingManager*> managerRef = _this.getReferenceUnsafeStaticCast();
	Reference<SceneObject*> markerRef = container;
	Reference<CreatureObject*> playerRef = player;

	Core::getTaskManager()->scheduleTask([managerRef, markerRef, playerRef] () {
		if (managerRef == nullptr || markerRef == nullptr || playerRef == nullptr)
			return;

		Locker lock(playerRef);

		Locker clock(markerRef, playerRef);

		managerRef->removeMarker(playerRef, markerRef);
	}, "RemoveMarkerLambda", 100);

	return 1;
}

// Function is triggered by player movement
void FishingManagerImplementation::checkFishingOnPositionUpdate(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	Locker lock(player);

	ManagedReference<FishingSession*> fishingSession = player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();

	if (fishingSession == nullptr || player->isSwimming()) {
		stopFishing(player, 0, true);
		return;
	}

	int fishingState = fishingSession->getFishingState();

	if (fishingState == FishingManager::NOTFISHING) {
		stopFishing(player, 0, true);
		return;
	}

	ManagedReference<SceneObject*> marker = fishingSession->getMarker();

	if (marker == nullptr) {
		stopFishing(player, 0, true);
		return;
	}

	float distance = player->getDistanceTo(marker);

	// Player has moved and bobber is either too far or player got too close and scared the fish
	if ((distance < 2.f && fishingState < REELING) || distance > 20.f) {
		stopFishing(player, 0, true);
		return;
	// Player has moved but is still in valid range but not close enough to catch the fish
	} else if (fishingState < REELING || distance > 2.f) {
		return;
	}

	success(player, fishingSession->getFish(), marker, 0);
}

String FishingManagerImplementation::getTime() {
	time_t seconds;
	time(&seconds);
	String result = ctime(&seconds);
	result = result + " PDT";
	return result;
}

String FishingManagerImplementation::getPropertyString(int amount) {
	String msg = "";

	for (int i = 1; i < amount; ++i) {
		msg += "*";
	}

	return msg;
}