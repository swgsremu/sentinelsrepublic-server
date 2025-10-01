/*
 * MissionTerminalImplementation.cpp
 *
 *  Created on: 03/05/11
 *      Author: polonel
 */

#include "server/zone/objects/tangible/terminal/mission/MissionTerminal.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/managers/city/CityManager.h"
#include "server/zone/managers/city/CityRemoveAmenityTask.h"
#include "server/zone/objects/player/sessions/SlicingSession.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/managers/mission/CardinalDirection.h"

void MissionTerminalImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	TerminalImplementation::fillObjectMenuResponse(menuResponse, player);

	ManagedReference<CityRegion*> city = player->getCityRegion().get();

	if (city != nullptr && city->isMayor(player->getObjectID()) && getParent().get() == nullptr) {

		menuResponse->addRadialMenuItem(72, 3, "@city/city:mt_remove"); // Remove

		menuResponse->addRadialMenuItem(73, 3, "@city/city:align"); // Align
		menuResponse->addRadialMenuItemToRadialID(73, 74, 3, "@city/city:north"); // North
		menuResponse->addRadialMenuItemToRadialID(73, 75, 3, "@city/city:east"); // East
		menuResponse->addRadialMenuItemToRadialID(73, 76, 3, "@city/city:south"); // South
		menuResponse->addRadialMenuItemToRadialID(73, 77, 3, "@city/city:west"); // West
	}
	// Add mission direction menu options for mission terminals
	if (getTerminalName() == "@terminal_name:terminal_mission") {
		menuResponse->addRadialMenuItem(80, 3, "@ui_radial:terminal_mission_set_direction"); // Set Mission Direction
		menuResponse->addRadialMenuItemToRadialID(80, 81, 3, "@ui_radial:terminal_mission_north"); // North
		menuResponse->addRadialMenuItemToRadialID(80, 82, 3, "@ui_radial:terminal_mission_northeast"); // Northeast
		menuResponse->addRadialMenuItemToRadialID(80, 83, 3, "@ui_radial:terminal_mission_east"); // East
		menuResponse->addRadialMenuItemToRadialID(80, 84, 3, "@ui_radial:terminal_mission_southeast"); // Southeast
		menuResponse->addRadialMenuItemToRadialID(80, 85, 3, "@ui_radial:terminal_mission_south"); // South
		menuResponse->addRadialMenuItemToRadialID(80, 86, 3, "@ui_radial:terminal_mission_southwest"); // Southwest
		menuResponse->addRadialMenuItemToRadialID(80, 87, 3, "@ui_radial:terminal_mission_west"); // West
		menuResponse->addRadialMenuItemToRadialID(80, 88, 3, "@ui_radial:terminal_mission_northwest"); // Northwest
		menuResponse->addRadialMenuItemToRadialID(80, 89, 3, "@ui_radial:terminal_mission_random"); // Random
	}
}

int MissionTerminalImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	ManagedReference<CityRegion*> city = player->getCityRegion().get();

	if (selectedID == 69 && player->hasSkill("combat_smuggler_slicing_01")) {
		if (isBountyTerminal())
			return 0;

		if (city != nullptr && !city->isClientRegion() && city->isBanned(player->getObjectID())) {
			player->sendSystemMessage("@city/city:banned_services"); // You are banned from using this city's services.
			return 0;
		}

		if (player->containsActiveSession(SessionFacadeType::SLICING)) {
			player->sendSystemMessage("@slicing/slicing:already_slicing");
			return 0;
		}

		if (!player->checkCooldownRecovery("slicing.terminal")) {
			StringIdChatParameter message;
			message.setStringId("@slicing/slicing:not_yet"); // You will be able to hack the network again in %DI seconds.
			message.setDI(player->getCooldownTime("slicing.terminal")->getTime() - Time().getTime());
			player->sendSystemMessage(message);
			return 0;
		}

		//Create Session
		ManagedReference<SlicingSession*> session = new SlicingSession(player);
		session->initalizeSlicingMenu(player, _this.getReferenceUnsafeStaticCast());

		return 0;

	} else if (selectedID == 72) {

		if (city != nullptr && city->isMayor(player->getObjectID())) {
			CityRemoveAmenityTask* task = new CityRemoveAmenityTask(_this.getReferenceUnsafeStaticCast(), city);
			task->execute();

			player->sendSystemMessage("@city/city:mt_removed"); // The object has been removed from the city.
		}

		return 0;

	} else if (selectedID == 74 || selectedID == 75 || selectedID == 76 || selectedID == 77) {

		CityManager* cityManager = getZoneServer()->getCityManager();
		cityManager->alignAmenity(city, player, _this.getReferenceUnsafeStaticCast(), selectedID - 74);

		return 0;

	} else if (selectedID >= 81 && selectedID <= 89) {
		// Handle ranger direction menu options
		CardinalDirection direction = CardinalDirection::RANDOM;
		
		switch (selectedID) {
			case 81: direction = CardinalDirection::NORTH; break;
			case 82: direction = CardinalDirection::NORTHEAST; break;
			case 83: direction = CardinalDirection::EAST; break;
			case 84: direction = CardinalDirection::SOUTHEAST; break;
			case 85: direction = CardinalDirection::SOUTH; break;
			case 86: direction = CardinalDirection::SOUTHWEST; break;
			case 87: direction = CardinalDirection::WEST; break;
			case 88: direction = CardinalDirection::NORTHWEST; break;
			case 89: direction = CardinalDirection::RANDOM; break; // Random
		}
		
		// Store the selected direction in the player's screenPlayData
		Reference<PlayerObject*> ghost = player->getSlottedObject("ghost").castTo<PlayerObject*>();
		if (ghost != nullptr) {
			ghost->setScreenPlayData("mission", "mission_direction", String::valueOf((int)direction));
		}
		
		String directionName = "";
		switch (direction) {
			case CardinalDirection::NORTH: directionName = "North"; break;
			case CardinalDirection::NORTHEAST: directionName = "Northeast"; break;
			case CardinalDirection::EAST: directionName = "East"; break;
			case CardinalDirection::SOUTHEAST: directionName = "Southeast"; break;
			case CardinalDirection::SOUTH: directionName = "South"; break;
			case CardinalDirection::SOUTHWEST: directionName = "Southwest"; break;
			case CardinalDirection::WEST: directionName = "West"; break;
			case CardinalDirection::NORTHWEST: directionName = "Northwest"; break;
			case CardinalDirection::RANDOM: directionName = "Random"; break;
		}
		
		player->sendSystemMessage("Mission direction set to: " + directionName);
		return 0;
	}

	return TangibleObjectImplementation::handleObjectMenuSelect(player, selectedID);
}

String MissionTerminalImplementation::getTerminalName() {
	String name = "@terminal_name:terminal_mission";

	if (terminalType == "artisan" || terminalType == "entertainer" || terminalType == "bounty" || terminalType == "imperial" || terminalType == "rebel" || terminalType == "scout")
		name = name + "_" + terminalType;

	return name;
}
