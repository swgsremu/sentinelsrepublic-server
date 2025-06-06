/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DISMOUNTCOMMAND_H_
#define DISMOUNTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/intangible/ControlDevice.h"
#include "templates/creature/SharedCreatureObjectTemplate.h"
#include "server/zone/packets/object/DataTransform.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage3.h"

class DismountCommand : public QueueCommand {
	Vector<uint32> restrictedBuffCRCs;
	uint32 gallopCRC;

public:
	DismountCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
		gallopCRC = STRING_HASHCODE("gallop");

		restrictedBuffCRCs.add(gallopCRC); // Remove the old buff off of any players on dismount
		restrictedBuffCRCs.add(STRING_HASHCODE("burstrun"));
		restrictedBuffCRCs.add(STRING_HASHCODE("retreat"));
		restrictedBuffCRCs.add(BuffCRC::JEDI_FORCE_RUN_1);
		restrictedBuffCRCs.add(BuffCRC::JEDI_FORCE_RUN_2);
		restrictedBuffCRCs.add(BuffCRC::JEDI_FORCE_RUN_3);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		if (!creature->hasState(CreatureState::RIDINGMOUNT)) {
			return INVALIDSTATE;
		}

		if (!creature->checkCooldownRecovery("mount_dismount")) {
			return GENERALERROR;
		}

		auto templateData = creature->getObjectTemplate();

		if (templateData == nullptr) {
			return GENERALERROR;
		}

		auto playerTemplate = dynamic_cast<SharedCreatureObjectTemplate*>(templateData);

		if (playerTemplate == nullptr) {
			return GENERALERROR;
		}

		float mountedSpeed = creature->getRunSpeed();

		// Remove Mounted combat slow from player
		creature->removeMountedCombatSlow(false);

		ManagedReference<SceneObject*> mount = creature->getParent().get();

		// Handle dismount, removal of gallop and storing of Jetpacks
		if (mount != nullptr && mount->isCreatureObject()) {
			handleMount(creature, mount);
		}

		creature->clearState(CreatureState::RIDINGMOUNT);

		// reapply speed buffs if they exist
		for (int i = 0; i < restrictedBuffCRCs.size(); i++) {
			uint32 buffCRC = restrictedBuffCRCs.get(i);

			if (creature->hasBuff(buffCRC)) {
				ManagedReference<Buff*> buff = creature->getBuff(buffCRC);

				if (buff != nullptr) {
					Locker lock(buff, creature);

					buff->applyAllModifiers();
				}
			}
		}

		SpeedMultiplierModChanges* changeBuffer = creature->getSpeedMultiplierModChanges();
		int bufferSize = changeBuffer->size();

		while (changeBuffer->size() > 4) {
			changeBuffer->remove(0);
		}

		changeBuffer->add(SpeedModChange(creature->getRunSpeed()));

 		// Reset Force Sensitive control mods to default.
		creature->updateSpeedAndAccelerationMods();

		// Update players stats in the database
		creature->updateToDatabase();

		// Update dismount timer
		creature->updateCooldownTimer("mount_dismount", 2000);
		creature->setNextAllowedMoveTime(500);

		creature->updateRunSpeed();

		// Client manipulates player height when dismounting, reset the player height sent to the client
		auto height = creature->getHeight();

		if (height != 1.f) {
			CreatureObjectDeltaMessage3* delta3 = new CreatureObjectDeltaMessage3(creature);

			if (delta3 != nullptr) {
				delta3->addFloatUpdate(0x0E, 1.f);
				delta3->addFloatUpdate(0x0E, height);
				delta3->close();

				creature->sendMessage(delta3);
			}
		}

		return SUCCESS;
	}

	void handleMount(CreatureObject* creature, SceneObject* mount) const {
		if (creature == nullptr || mount == nullptr) {
			return;
		}

		auto vehicle = mount->asCreatureObject();

		if (vehicle == nullptr) {
			return;
		}

		auto zone = vehicle->getZone();

		if (zone == nullptr) {
			return;
		}

		auto ghost = creature->getPlayerObject();

		if (ghost == nullptr) {
			return;
		}

		auto playerValidated = ghost->getLastValidatedPosition();

		if (playerValidated == nullptr) {
			return;
		}

		Locker clocker(vehicle, creature);

		// Handle dismounting player
		if (vehicle == creature->getParent().get()) {
			// Player will be sent to the vehicles position in the world
			Vector3 validatedPosition = playerValidated->getPosition();

			float vehicleSpeed = vehicle->getCurrentSpeed();

			if (vehicleSpeed > 0) {
				// Get a position 1m back from the vehicles position
				float angle = vehicle->getDirection()->getRadians();

				angle = M_PI / 2 - angle;

				if (angle < 0) {
					float a = M_PI + angle;
					angle = M_PI + a;
				}

				validatedPosition.setX(validatedPosition.getX() + (Math::cos(angle) * -1.f));
				validatedPosition.setY(validatedPosition.getY() + (Math::sin(angle) * -1.f));
			}

			auto planetManager = zone->getPlanetManager();

			if (planetManager != nullptr) {
				auto terrainManager = planetManager->getTerrainManager();

				if (terrainManager != nullptr) {
					IntersectionResults intersections;
					CollisionManager::getWorldFloorCollisions(validatedPosition.getX(), validatedPosition.getY(), zone, &intersections, (CloseObjectsVector*)creature->getCloseObjects());
					validatedPosition.setZ(planetManager->findClosestWorldFloor(validatedPosition.getX(), validatedPosition.getY(), validatedPosition.getZ(), creature->getSwimHeight(), &intersections, (CloseObjectsVector*)creature->getCloseObjects()));
				}
			}

			// Transfer them into the zone
			zone->transferObject(creature, -1, false, false, false);

			vehicle->clearState(CreatureState::MOUNTEDCREATURE);

			// Update the players position
			creature->teleport(validatedPosition.getX(), validatedPosition.getZ(), validatedPosition.getY(), 0);

			/*
			// debug markers
			Reference<SceneObject*> movementMarker = creature->getZoneServer()->createObject(STRING_HASHCODE("object/path_waypoint/path_waypoint.iff"), 0);

			Locker moveLock(movementMarker, creature);
			movementMarker->initializePosition(validatedPosition.getX(), validatedPosition.getZ(), validatedPosition.getY());
			zone->transferObject(movementMarker, -1, true);
			moveLock.release();
			// END debug markers
			*/

			ManagedReference<PlayerManager*> playerManager = server->getPlayerManager();

			if (playerManager != nullptr) {
				playerManager->updateSwimmingState(creature, validatedPosition.getZ());
			}
		}

		// Remove gallop from mounts
		if (vehicle->hasBuff(gallopCRC)) {
			removeMountBuffs(vehicle);
		}

		// Store Jetpack
		ManagedReference<ControlDevice*> device = vehicle->getControlDevice().get();

		if (device != nullptr && vehicle->getServerObjectCRC() == 0x32F87A54) { // Auto-store jetpack on dismount.
			device->storeObject(creature);
			creature->sendSystemMessage("@pet/pet_menu:jetpack_dismount"); // "You have been dismounted from the jetpack, and it has been stored."
		}

		if (vehicle->getParentID() == 0) {
			vehicle->setCurrentSpeed(0.f);
			vehicle->incrementMovementCounter();

			auto data = new DataTransform(vehicle);

			if (data != nullptr) {
				vehicle->broadcastMessage(data, false);
			}
		}
	}

	void removeMountBuffs(CreatureObject* vehicle) const {
		if (vehicle == nullptr)
			return;

		ManagedReference<Buff*> buff = vehicle->getBuff(gallopCRC);

		if (buff == nullptr)
			return;

		Reference<CreatureObject*> vehicleRef = vehicle;

		Core::getTaskManager()->executeTask([vehicleRef, buff]() {
			Locker lock(vehicleRef);
			Locker buffLocker(buff, vehicleRef);

			buff->removeAllModifiers();
		}, "RemoveGallopModsLambda");
	}
};

#endif // DISMOUNTCOMMAND_H_
