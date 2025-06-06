#include "engine/engine.h"

#include "server/zone/objects/creature/buffs/PlayerVehicleBuff.h"

#include "server/zone/objects/creature/CreatureObject.h"

#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/ZoneServer.h"



void PlayerVehicleBuffImplementation::applyAllModifiers() {

	if (!modsApplied) {
		applyAttributeModifiers();
		applySkillModifiers();
		applyStates();
		modsApplied = true;

		updateRiderSpeeds();
	}
}

void PlayerVehicleBuffImplementation::removeAllModifiers() {

	if (modsApplied) {
		removeAttributeModifiers();
		removeSkillModifiers();
		removeStates();
		modsApplied = false;

		updateRiderSpeeds();
	}
}

void PlayerVehicleBuffImplementation::activate(bool applyModifiers) {
		BuffImplementation::activate(applyModifiers);
		//Send start message to mount rider
		if (!startMessage.isEmpty()) {

			ManagedReference<CreatureObject*> rider = creature.get()->getSlottedObject("rider").castTo<CreatureObject*>();

			if(rider != nullptr) {
				rider->sendSystemMessage(startMessage);
			}
		}

}

void PlayerVehicleBuffImplementation::deactivate(bool removeModifiers) {
		BuffImplementation::deactivate(removeModifiers);
		//Send end message to mount rider
		if (!endMessage.isEmpty()) {

			ManagedReference<CreatureObject*> rider = creature.get()->getSlottedObject("rider").castTo<CreatureObject*>();

			if(rider != nullptr) {
				rider->sendSystemMessage(endMessage);
			}
		}

}

void PlayerVehicleBuffImplementation::updateRiderSpeeds() {
	ManagedReference<CreatureObject*> vehicle = creature.get();

	if (vehicle == nullptr) {
		return;
	}

	Core::getTaskManager()->executeTask([vehicle] () {
		if (vehicle == nullptr) {
			return;
		}

		Locker lock(vehicle);

		// get vehicle speed
		float newSpeed = vehicle->getRunSpeed();

		// get animal mount speeds
		if (vehicle->isMount()) {
			PetManager* petManager = vehicle->getZoneServer()->getPetManager();

			if (petManager != nullptr) {
				newSpeed = petManager->getMountedRunSpeed(vehicle);
			}
		}

		// add speed multiplier mod
		newSpeed *= vehicle->getSpeedMultiplierMod();

		// Update Vehicles Speed
		vehicle->setRunSpeed(newSpeed);

		ManagedReference<CreatureObject*> rider = vehicle->getSlottedObject("rider").castTo<CreatureObject*>();

		// Update riders change buffer
		if (rider != nullptr) {
			if (rider->isRidingMount()) {
				Locker rideClock(rider, vehicle);

				// Speed hack buffer
				SpeedMultiplierModChanges* changeBuffer = rider->getSpeedMultiplierModChanges();
				const int bufferSize = changeBuffer->size();

				// Drop old change off the buffer
				if (bufferSize > 5) {
					changeBuffer->remove(0);
				}

				// Add a fake "skillmod" change
				changeBuffer->add(SpeedModChange(newSpeed / rider->getRunSpeed()));

				// Commit changebuffer
				rider->updateToDatabase();
			}

			// Send riders speed update
			rider->updateRunSpeed();
		}
	}, "UpdateRiderSpeedsLambda");
}
