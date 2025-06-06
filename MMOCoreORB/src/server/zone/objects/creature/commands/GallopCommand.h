/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GALLOPCOMMAND_H_
#define GALLOPCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/buffs/GallopBuff.h"
#include "server/zone/objects/creature/events/GallopNotifyAvailableEvent.h"

class GallopCommand : public QueueCommand {
public:

	GallopCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<SceneObject*> parent = creature->getParent().get();

		if (parent == nullptr || !parent->isMount()) {
			creature->sendSystemMessage("@combat_effects:not_mounted"); // You cannot perform this command without a mount.
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();

		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto mount = parent->asCreatureObject();

		if (mount == nullptr) {
			return GENERALERROR;
		}

		Locker crossLocker(mount, creature);

		uint32 crc = STRING_HASHCODE("gallop");

		if (mount->hasBuff(crc) || creature->hasBuff(crc)) {
			creature->sendSystemMessage("@combat_effects:already_galloping"); // You are already galloping!
			return GENERALERROR;
		}

		if (!mount->checkCooldownRecovery("gallop")) {
			creature->sendSystemMessage("@combat_effects:mount_tired"); // Your mount is too tired to gallop.
			return GENERALERROR;
		}

		PetManager* petManager = zoneServer->getPetManager();
		ManagedReference<PetControlDevice*> pcd = mount->getControlDevice().get().castTo<PetControlDevice*>();

		if (petManager == nullptr || pcd == nullptr) {
			return GENERALERROR;
		}

		SharedObjectTemplate* objectTemplate = pcd->getObjectTemplate();

		if (objectTemplate == nullptr)
			return GENERALERROR;

		MountSpeedData* mountSpeedData = petManager->getMountSpeedData(objectTemplate->getAppearanceFilename());

		if (mountSpeedData == nullptr)
			return GENERALERROR;

		int duration = mountSpeedData->getGallopDuration();
		float magnitude = mountSpeedData->getGallopSpeedMultiplier();
		int cooldown = mountSpeedData->getGallopCooldown();

		StringIdChatParameter startStringId("combat_effects", "gallop_start"); // Your mount runs as fast as it can.
		StringIdChatParameter endStringId("combat_effects", "gallop_stop"); // Your mount is winded and slows down.

		ManagedReference<GallopBuff*> buff = new GallopBuff(mount, crc, duration);

		Locker locker(buff);

		buff->setSpeedMultiplierMod(magnitude);
		buff->setAccelerationMultiplierMod(magnitude);
		buff->setStartMessage(startStringId);
		buff->setEndMessage(endStringId);

		mount->addBuff(buff);

		mount->updateCooldownTimer("gallop", (cooldown + duration) * 1000);

		Reference<GallopNotifyAvailableEvent*> task = new GallopNotifyAvailableEvent(mount);
		mount->addPendingTask("gallop_notify", task, (cooldown + duration) * 1000);

		return SUCCESS;
	}

};

#endif //GALLOPCOMMAND_H_
