/*
	 Copyright <SWGEmu>
	 See file COPYING for copying conditions.
*/

#ifndef DROID_H_
#define DROID_H_

#include "server/zone/managers/ship/ShipManager.h"
#include "server/zone/managers/ship/DroidCommandData.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/objects/creature/events/DroidCommandNotifyAvailableEvent.h"
#include "server/zone/packets/ship/ShipObjectMessage1.h"
#include "server/zone/packets/ship/ShipObjectMessage3.h"

class DroidCommand : public QueueCommand {
public:
	DroidCommand(const String &name, ZoneProcessServer *server)
		: QueueCommand(name, server)
	{
	}

	int doQueueCommand(CreatureObject *creature, const uint64 &target, const UnicodeString &arguments) const
	{
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		String args = arguments.toString();
		uint32 argsHash = args.hashCode();

		if (args.length() < 1)
			return GENERALERROR;

		ShipManager *shipMan = ShipManager::instance();

		if (shipMan == nullptr)
			return GENERALERROR;

		ManagedReference<ShipObject *> ship = cast<ShipObject *>(creature->getRootParent());

		if (ship == nullptr)
			return GENERALERROR;

		auto zoneServer = creature->getZoneServer();

		if (zoneServer == nullptr)
			return GENERALERROR;

		// no droid interface
		if (!ship->isComponentInstalled(Components::DROID_INTERFACE)) {
			creature->sendSystemMessage("@space/space_interaction:droid_command_fail_no_interface");
			return GENERALERROR;
		}

		// droid interface is disabled
		if (!ship->isComponentFunctional(Components::DROID_INTERFACE)) {
			creature->sendSystemMessage("@space/space_interaction:droid_command_fail_interface_disabled");
			return GENERALERROR;
		}

		// droid commands on cooldown
		if (!ship->isReadyForDroidCommand()) {
			StringIdChatParameter droidDelayMsg;
			droidDelayMsg.setStringId("space/space_interaction", "droid_command_delay");
			droidDelayMsg.setDI(ship->timeUntilNextDroidCommand());
			creature->sendSystemMessage(droidDelayMsg);
			return GENERALERROR;
		}

		auto droidControlDevice = cast<IntangibleObject *>(zoneServer->getObject(ship->getShipDroidID()).get());

		// no droid
		if (droidControlDevice == nullptr) {
			creature->sendSystemMessage("@space/space_interaction:droid_command_fail_no_droid");
			return GENERALERROR;
		}

		if (!ship->hasDroidCommand(argsHash)) {
			creature->sendSystemMessage("@space/space_interaction:droid_command_fail_no_command");
			return GENERALERROR;
		}

		DroidCommandData *commandData = shipMan->getDroidCommandData(argsHash);

		if (commandData == nullptr)
			return GENERALERROR;

		float delayModifier = commandData->getDelayModifier();
		int componentType = commandData->getComponentType();
		float energyEfficiency = commandData->getEnergyEfficiency();
		float generalEfficiency = commandData->getGeneralEfficiency();
		float damage = commandData->getDamage();
		float frontReinforceRatio = commandData->getFrontReinforceRatio();
		float capacitorReinforcePercentage = commandData->getCapacitorReinforcePercentage();
		float frontShieldRatio = commandData->getFrontShieldRatio();
		StringIdChatParameter stringID = commandData->getStringID();

		int endComponent = componentType;

		if (componentType == 12)
			endComponent = 20;

		auto hitpointsMap = ship->getCurrentHitpointsMap();

		if (hitpointsMap == nullptr)
			return GENERALERROR;

		auto armorMap = ship->getCurrentArmorMap();

		if (armorMap == nullptr)
			return GENERALERROR;

		Locker clocker(ship, creature);

		float hitpoints = hitpointsMap->get(componentType);
		float armor = armorMap->get(componentType);

		// component too damaged
		if ((hitpoints + armor) < damage) {
			StringIdChatParameter damagedComponentMsg;
			damagedComponentMsg.setStringId("space/space_interaction", "droid_component_too_damaged");
			damagedComponentMsg.setTO(ship->getComponentNameMap()->get(componentType));
			creature->sendSystemMessage(damagedComponentMsg);
			return GENERALERROR;
		}

		// apply overloads/tuning
		for (int i = componentType; i <= endComponent; ++i) {
			if (energyEfficiency != 0)
				ship->setEnergyEfficiency(i, energyEfficiency);

			if (generalEfficiency != 0)
				ship->setEfficiency(i, generalEfficiency);

			if (damage > 0) {
				armor = armorMap->get(i);
				hitpoints = hitpointsMap->get(i);
				float damageToInflict = damage;

				if (damageToInflict > armor) {
					damageToInflict -= armor;
					armor = 0.f;
					hitpoints -= damageToInflict;
				} else {
					armor -= damageToInflict;
				}

				ship->setComponentArmor(i, armor);
				ship->setComponentHitpoints(i, hitpoints);
			}
		}

		// capacitor to shield shunt
		if (capacitorReinforcePercentage > 0) {
			float energyPool = ship->getCapacitorEnergy();

			ship->setFrontShield(Math::min(ship->getFrontShield() + energyPool / 2.0f, ship->getMaxFrontShield()));
			ship->setRearShield(Math::min(ship->getRearShield() + energyPool / 2.0f, ship->getMaxRearShield()));
			ship->setCapacitorEnergy(energyPool * (1.0f - capacitorReinforcePercentage), false, nullptr, nullptr);
		}

		// shield reinforcement
		if (frontReinforceRatio > 0) {
			if (frontReinforceRatio > 1.0f) {	// power from rear shield to front shield
				frontReinforceRatio = frontReinforceRatio - 1.0f;
				float rearShield = ship->getRearShield();

				ship->setRearShield(rearShield * (1.0f - frontReinforceRatio));
				ship->setFrontShield(Math::min(ship->getFrontShield() + rearShield * frontReinforceRatio, ship->getMaxFrontShield()));
			} else {	// power from front shield to rear shield
				frontReinforceRatio = 1.0f - frontReinforceRatio;
				float frontShield = ship->getFrontShield();

				ship->setFrontShield(frontShield * (1.0f - frontReinforceRatio));
				ship->setRearShield(Math::min(ship->getRearShield() + frontShield * frontReinforceRatio, ship->getMaxRearShield()));
			}
		}

		// shield adjustment
		if (frontShieldRatio > 0) {
			float totalShieldEnergy = ship->getMaxFrontShield() + ship->getMaxRearShield();
			float frontShieldEnergy = totalShieldEnergy / 2.0f * frontShieldRatio;

			ship->setFrontShieldMax(frontShieldEnergy);
			ship->setRearShieldMax(totalShieldEnergy - frontShieldEnergy);
		}

		// broadcast changes to the pilot for updated displays
		ShipObjectMessage1 *ship1 = new ShipObjectMessage1(ship);
		ShipObjectMessage3 *ship3 = new ShipObjectMessage3(ship);

		ship->sendMembersBaseMessage(ship1, true);
		ship->sendMembersBaseMessage(ship3, true);

		// mute/unmute droid is a special case
		String muteDroid = "droidcommand_mutedroid";

		if (argsHash == muteDroid.hashCode()) {
			if (ship->isDroidMuted())
				ship->setDroidFeedback(true);
			else {
				ship->setDroidFeedback(false);
				creature->sendSystemMessage(stringID);
			}
		}

		// apply delay
		ship->setDroidCommandDelay(delayModifier * ship->getDroidCommandSpeed());

		Reference<DroidCommandNotifyAvailableEvent *> task = new DroidCommandNotifyAvailableEvent(creature);

		creature->addPendingTask("droid_command_notify", task, (delayModifier * ship->getDroidCommandSpeed() * 1000));

		// do droid chatter if droid's not muted
		if (!ship->isDroidMuted()) {
			ship->droidChatter(creature, stringID);
		}

		return SUCCESS;
	}
};

#endif // DROID_H_

