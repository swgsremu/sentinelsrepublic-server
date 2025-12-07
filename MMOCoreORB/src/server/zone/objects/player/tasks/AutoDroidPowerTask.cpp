/*
 * Copyright (c) 2024 - on Wasted Potential Studios.
 * Proprietary code and work not for distribution.
 */

#include "server/zone/objects/player/tasks/AutoDroidPowerTask.h"

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/ai/DroidObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "system/lang/StringBuffer.h"
#include "system/thread/Locker.h"

void AutoDroidPowerTask::run() {
	ManagedReference<server::zone::objects::creature::CreatureObject*> ownerRef = owner.get();
	ManagedReference<server::zone::objects::creature::ai::DroidObject*> droidRef = droid.get();

	if (ownerRef == nullptr || droidRef == nullptr)
		return;

	Locker ownerLock(ownerRef.get());
	Locker droidLock(droidRef.get(), ownerRef.get());

	ownerRef->removePendingTask("auto_droid_power");

	PlayerObject* ghost = ownerRef->getPlayerObject();

	if (ghost == nullptr || !ghost->isAutoDroidPowerEnabled())
		return;

	if (droidRef->getZone() == nullptr)
		return;

	float percentPower = 100.0f;

	if (droidRef->getMaxPowerLevel() > 0)
		percentPower = ((float)droidRef->getPowerLevel() / (float)droidRef->getMaxPowerLevel()) * 100.0f;

	if (percentPower > 5.0f)
		return;

	int previousPower = droidRef->getPowerLevel();

	droidRef->rechargeFromBattery(ownerRef.get());

	if (droidRef->getPowerLevel() > previousPower) {
		StringBuffer ownerMessage;
		ownerMessage << droidRef->getDisplayedName() << " automatically recharged using a droid battery.";
		ownerRef->sendSystemMessage(ownerMessage.toString());
		return;
	}

	ownerRef->sendSystemMessage("You do not have a droid battery in your inventory to recharge your droid.");
}
