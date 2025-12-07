#include <cmath>
#include <algorithm>
/*
 * EntertainingSessionImplementation.cpp
 *
 *  Created on: 10/21/2025
 *      Author: Joseph Ridder
 */

#include "server/zone/objects/player/sessions/EntertainingSession.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/managers/skill/Performance.h"
#include "server/zone/managers/skill/PerformanceManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/events/EntertainingSessionTask.h"
#include "server/zone/objects/player/EntertainingObserver.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/FactionStatus.h"
#include "server/zone/objects/tangible/Instrument.h"
#include "server/zone/packets/object/Flourish.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage6.h"
#include "server/zone/objects/mission/MissionObject.h"
#include "server/zone/objects/mission/EntertainerMissionObjective.h"
#include "server/zone/objects/creature/buffs/PerformanceBuff.h"
#include "server/zone/objects/creature/buffs/PerformanceBuffType.h"
#include "server/zone/objects/mission/MissionTypes.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/chat/ChatManager.h"
#include "server/zone/Zone.h"
#include "server/zone/packets/scene/PlayClientEffectLocMessage.h"
#include "system/lang/StringBuffer.h"

namespace {
	const float ENTERTAINER_MAX_BUFF_DURATION_MINUTES = 120.0f + (10.0f / 60.0f);
	const int ENTERTAINER_AUTO_STAGE_NONE = 0;
	const int ENTERTAINER_AUTO_STAGE_DANCE = 1;
	const int ENTERTAINER_AUTO_STAGE_MUSIC = 2;
	const int ENTERTAINER_AUTO_ACTION_NONE = 0;
	const int ENTERTAINER_AUTO_ACTION_ADVANCE_STAGE = 1;
	const int ENTERTAINER_AUTO_ACTION_DISABLE = 2;

	int computeAutoCycleTicks(CreatureObject* entertainer, Performance* performance, bool isDanceStage) {
		if (entertainer == nullptr || performance == nullptr)
			return 0;

		float buffAcceleration = 1.0f + (float) entertainer->getSkillMod("accelerate_entertainer_buff") / 100.0f;

		if (buffAcceleration <= 0.0f)
			buffAcceleration = 1.0f;

		float durationPerTick = 2.0f * buffAcceleration;

		if (durationPerTick <= 0.0f)
			durationPerTick = 2.0f;

		int ticksForDuration = (int) std::ceil(ENTERTAINER_MAX_BUFF_DURATION_MINUTES / durationPerTick);

		float maxBuffStrength = 0.0f;

		if (isDanceStage)
			maxBuffStrength = (float) entertainer->getSkillMod("healing_dance_mind");
		else
			maxBuffStrength = (float) entertainer->getSkillMod("healing_music_mind");

		if (maxBuffStrength > 125.0f)
			maxBuffStrength = 125.0f;

		float factionPerkStrength = entertainer->getSkillMod("private_faction_buff_mind");

		ManagedReference<BuildingObject*> building = cast<BuildingObject*>(entertainer->getRootParent());

		if (building != nullptr && factionPerkStrength > 0 && building->isPlayerRegisteredWithin(entertainer->getObjectID())) {
			unsigned int buildingFaction = building->getFaction();
			unsigned int entFaction = entertainer->getFaction();

			if (entFaction != 0 && entFaction == buildingFaction && entertainer->getFactionStatus() == FactionStatus::OVERT)
				maxBuffStrength += factionPerkStrength;
		}

		int ticksForStrength = 0;

		if (maxBuffStrength > 0.0f) {
			float perTickStrength = (float) performance->getHealShockWound();

			if (perTickStrength <= 0.0f)
				perTickStrength = maxBuffStrength;

			ticksForStrength = (int) std::ceil(maxBuffStrength / perTickStrength);
		}

		int ticks = std::max(ticksForDuration, ticksForStrength);

		if (ticks <= 0)
			ticks = std::max(1, ticksForDuration);

		return ticks + 1;
	}
}

void EntertainingSessionImplementation::doEntertainerPatronEffects() {
	ManagedReference<CreatureObject*> creo = entertainer.get();

	if (creo == nullptr)
		return;

	if (performanceIndex == 0)
		return;

	Locker locker(creo);

	SkillManager* skillManager = creo->getZoneServer()->getSkillManager();
	PerformanceManager* performanceManager = skillManager->getPerformanceManager();

	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	ManagedReference<Instrument*> instrument = creo->getPlayableInstrument();

	float woundHealingSkill = 0.0f;
	float playerShockHealingSkill = 0.0f;
	float buildingShockHealingSkill = creo->getSkillMod("private_med_battle_fatigue");
	float factionPerkSkill = creo->getSkillMod("private_faction_mind_heal");

	if (isDancing()) {
		woundHealingSkill = (float) creo->getSkillMod("healing_dance_wound");
		playerShockHealingSkill = (float) creo->getSkillMod("healing_dance_shock");
	} else if (isPlayingMusic() && instrument != nullptr) {
		woundHealingSkill = (float) creo->getSkillMod("healing_music_wound");
		playerShockHealingSkill = (float) creo->getSkillMod("healing_music_shock");
	} else {
		cancelSession();
		return;
	}

	ManagedReference<BuildingObject*> building = cast<BuildingObject*>(creo->getRootParent());

	if (building != nullptr && factionPerkSkill > 0 && building->isPlayerRegisteredWithin(creo->getObjectID())) {
		unsigned int buildingFaction = building->getFaction();
		unsigned int healerFaction = creo->getFaction();

		if (healerFaction != 0 && healerFaction == buildingFaction && creo->getFactionStatus() == FactionStatus::OVERT) {
			woundHealingSkill += factionPerkSkill;
			playerShockHealingSkill += factionPerkSkill;
		}
	}

	int woundHeal = ceil(performance->getHealMindWound() * (woundHealingSkill / 100.0f));
	int shockHeal = ceil(performance->getHealShockWound() * ((playerShockHealingSkill + buildingShockHealingSkill) / 100.0f));

	healWounds(creo, woundHeal * (flourishCount + 1), shockHeal * (flourishCount + 1));

	if (patronDataMap.size() <= 0)
		return;

	ManagedReference<PlayerManager*> playerManager = creo->getZoneServer()->getPlayerManager();

	if (playerManager == nullptr)
		return;

	for (int i = 0; i < patronDataMap.size(); ++i) {
		ManagedReference<CreatureObject*> patron = patronDataMap.elementAt(i).getKey();

		if (patron == nullptr)
			continue;

		try {
			if (creo->isInRange(patron, PerformanceManager::HEAL_RANGE)) {
				healWounds(patron, woundHeal * (flourishCount + 1), shockHeal * (flourishCount + 1));
				increaseEntertainerBuff(patron);

			} else {
				Locker locker(patron, creo);

				if (isDancing()) {
					playerManager->stopWatch(patron, creo->getObjectID(), true, false, false, true);

					if (!patron->isListening())
						sendEntertainmentUpdate(patron, 0, "");
				} else if (isPlayingMusic()) {
						playerManager->stopListen(patron, creo->getObjectID(), true, false, false, true);

					if (!patron->isWatching())
						sendEntertainmentUpdate(patron, 0, "");
				}
			}

		} catch (Exception& e) {
			error("Unreported exception caught in EntertainingSessionImplementation::doEntertainerPatronEffects()");
		}
	}
}

bool EntertainingSessionImplementation::isInEntertainingBuilding(CreatureObject* creature) {
	ManagedReference<SceneObject*> root = creature->getRootParent();

	if (root != nullptr) {
		uint32 gameObjectType = root->getGameObjectType();

		switch (gameObjectType) {
		case SceneObjectType::RECREATIONBUILDING:
		case SceneObjectType::HOTELBUILDING:
		case SceneObjectType::THEATERBUILDING:
			return true;
		}
	}

	return false;
}

void EntertainingSessionImplementation::healWounds(CreatureObject* creature, float woundHeal, float shockHeal) {
	float amountHealed = 0;

	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	Locker clocker(creature, entertainer);

	if (!canGiveEntertainBuff())
		return;

	if (isInDenyServiceList(creature))
		return;

	if (shockHeal > 0 && creature->getShockWounds() > 0 && canHealBattleFatigue()) {
		creature->addShockWounds(-shockHeal, true, false);
		amountHealed += shockHeal;
	}
	if (woundHeal > 0 && (creature->getWounds(CreatureAttribute::MIND) > 0
			|| creature->getWounds(CreatureAttribute::FOCUS) > 0
			|| creature->getWounds(CreatureAttribute::WILLPOWER) > 0)) {
		creature->healWound(entertainer, CreatureAttribute::MIND, woundHeal, true, false);
		creature->healWound(entertainer, CreatureAttribute::FOCUS, woundHeal, true, false);
		creature->healWound(entertainer, CreatureAttribute::WILLPOWER, woundHeal, true, false);

		amountHealed += woundHeal;
	}

	clocker.release();

	if (entertainer->getGroup() != nullptr)
		addHealingXpGroup(amountHealed);
	else
		addHealingXp(amountHealed);

}

void EntertainingSessionImplementation::addHealingXpGroup(int xp) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	ManagedReference<GroupObject*> group = entertainer->getGroup();
	ManagedReference<PlayerManager*> playerManager = entertainer->getZoneServer()->getPlayerManager();

	for (int i = 0; i < group->getGroupSize(); ++i) {
		try {
			ManagedReference<CreatureObject *> groupMember = group->getGroupMember(i);

			if (groupMember != nullptr && groupMember->isPlayerCreature()) {
				Locker clocker(groupMember, entertainer);

				if (groupMember->isEntertaining() && groupMember->isInRange(entertainer, 40.0f)
					&& groupMember->hasSkill("social_entertainer_novice")) {
					String healxptype("entertainer_healing");

					if (playerManager != nullptr)
						playerManager->awardExperience(groupMember, healxptype, xp, true);
				}
			}
		} catch (Exception& e) {
			warning("exception in EntertainingSessionImplementation::addHealingXpGroup: " + e.getMessage());
		}
	}
}

void EntertainingSessionImplementation::activateAction() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	Locker locker(entertainer);

	if (!isDancing() && !isPlayingMusic()) {
		return; // don't tick action if they aren't doing anything
	}

	doEntertainerPatronEffects();
	awardEntertainerExperience();
	doPerformanceAction();


	int autoCycleAction = ENTERTAINER_AUTO_ACTION_NONE;

	if (autoCycleEnabled)
		autoCycleAction = handleAutoCycleTick();

	startTickTask();

	entertainer->debug("EntertainerEvent completed.");

	if (autoCycleAction != ENTERTAINER_AUTO_ACTION_NONE) {
		locker.release();
		if (autoCycleAction == ENTERTAINER_AUTO_ACTION_ADVANCE_STAGE)
			advanceAutoCycleStage();
		else if (autoCycleAction == ENTERTAINER_AUTO_ACTION_DISABLE)
			disableAutoCycle(true);
	}
}

void EntertainingSessionImplementation::startTickTask() {
	if (tickTask == nullptr) {
		tickTask = new EntertainingSessionTask(_this.getReferenceUnsafeStaticCast());
	}

	if (!tickTask->isScheduled()) {
		tickTask->schedule(10000);
	}
}

void EntertainingSessionImplementation::doPerformanceAction() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	if (performanceIndex == 0)
		return;

	Locker locker(entertainer);

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	ManagedReference<Instrument*> instrument = entertainer->getPlayableInstrument();

	if (!isDancing() && (!isPlayingMusic() || !instrument)) {
		cancelSession();
		return;
	}

	int actionDrain = entertainer->calculateCostAdjustment(CreatureAttribute::QUICKNESS, performance->getActionPointsPerLoop());

	if (entertainer->getHAM(CreatureAttribute::ACTION) <= actionDrain) {
		if (isDancing()) {
			stopDancing();
			entertainer->sendSystemMessage("@performance:dance_too_tired");
		}

		if (isPlayingMusic()) {
			stopMusic(true);
			entertainer->sendSystemMessage("@performance:music_too_tired");
		}
	} else {
		entertainer->inflictDamage(entertainer, CreatureAttribute::ACTION, actionDrain, false, true);
	}
}

void EntertainingSessionImplementation::stopPlaying() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	bool disableAutoAfterStop = false;
	bool keepSessionActive = false;

	{
		Locker locker(entertainer);

		if (!isPlayingMusic())
			return;

		keepSessionActive = autoCycleEnabled && autoCycleStageChanging;

		performanceIndex = 0;
		entertainer->setListenToID(0);

		entertainer->dropObserver(ObserverEventType::POSTURECHANGED, observer);
		entertainer->setPosture(CreaturePosture::UPRIGHT, true, true);

		if (isPerformingOutro())
			setPerformingOutro(false);

		ManagedReference<PlayerManager*> playerManager = entertainer->getZoneServer()->getPlayerManager();

		while (patronDataMap.size() > 0) {
			ManagedReference<CreatureObject*> patron = patronDataMap.elementAt(0).getKey();

			Locker clocker(patron, entertainer);

			playerManager->stopListen(patron, entertainer->getObjectID(), true, true, false);

			if (!patron->isWatching())
				sendEntertainmentUpdate(patron, 0, "");

			patronDataMap.drop(patron);
		}

		if (tickTask != nullptr && tickTask->isScheduled())
			tickTask->cancel();

		sendEntertainingUpdate(entertainer, 0, false);
		updateEntertainerMissionStatus(false, MissionTypes::MUSICIAN);

		entertainer->notifyObservers(ObserverEventType::STOPENTERTAIN, entertainer);

		if (!isDancing() && !isPlayingMusic()) {
			ManagedReference<PlayerObject*> entPlayer = entertainer->getPlayerObject();

			if (entPlayer != nullptr && entPlayer->getPerformanceBuffTarget() != 0)
				entPlayer->setPerformanceBuffTarget(0);

			if (!keepSessionActive)
				entertainer->dropActiveSession(SessionFacadeType::ENTERTAINING);
		}

		if (autoCycleEnabled && !autoCycleStageChanging)
			disableAutoAfterStop = true;
	}

	if (disableAutoAfterStop)
		disableAutoCycle(false);
}

void EntertainingSessionImplementation::stopMusic(bool skipOutro, bool bandStop, bool isBandLeader) {
	ManagedReference<CreatureObject*> player = entertainer.get();

	if (player == nullptr)
		return;

	if (isPerformingOutro() && !skipOutro)
		return;

	if (performanceIndex == 0)
		return;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();

	if (skipOutro) {
		performanceManager->performanceMessageToSelf(player, nullptr, "performance", "music_stop_self"); // You stop playing.
		performanceManager->performanceMessageToBand(player, nullptr, "performance", "music_stop_other"); // %TU stops playing.
		performanceManager->performanceMessageToBandPatrons(player, nullptr, "performance", "music_stop_other"); // %TU stops playing.
		stopPlaying();
	} else {
		Flourish* flourish = new Flourish(player, -1);
		player->broadcastMessage(flourish, true);

		setPerformingOutro(true);

		performanceManager->performanceMessageToSelf(player, nullptr, "performance", "music_prepare_stop_self"); // You prepare to stop playing.

		ManagedReference<EntertainingSession*> strongSess = _this.getReferenceUnsafeStaticCast();

		Core::getTaskManager()->scheduleTask([player, strongSess, bandStop, isBandLeader] {
			Locker lock(player);
			strongSess->clearOutro(bandStop, isBandLeader);
		}, "SetPerformingOutroTask", 15000);
	}
}

void EntertainingSessionImplementation::clearOutro(bool bandStop, bool isBandLeader) {
	ManagedReference<CreatureObject*> player = entertainer.get();

	if (player == nullptr)
		return;

	if (!isPerformingOutro())
		return;

	setPerformingOutro(false);

	if (performanceIndex == 0)
		return;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();

	if (bandStop && isBandLeader) {
		performanceManager->performanceMessageToSelf(player, nullptr, "performance", "music_stop_band_self"); // You stop the band.
		performanceManager->performanceMessageToBand(player, nullptr, "performance", "music_stop_band_members"); // %TU stops your band.
		performanceManager->performanceMessageToBandPatrons(player, nullptr, "performance", "music_stop_band_other"); // %TU's band stops playing.
	} else if (!bandStop) {
		performanceManager->performanceMessageToSelf(player, nullptr, "performance", "music_stop_self"); // You stop playing.
		performanceManager->performanceMessageToBand(player, nullptr, "performance", "music_stop_other"); // %TU stops playing.
		performanceManager->performanceMessageToBandPatrons(player, nullptr, "performance", "music_stop_other"); // %TU stops playing.
	}

	stopPlaying();
}

void EntertainingSessionImplementation::startDancing(int perfIndex) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	performanceIndex = perfIndex;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	Locker locker(entertainer);

	sendEntertainingUpdate(entertainer, performanceIndex, true);

	entertainer->sendSystemMessage("@performance:dance_start_self"); // You begin dancing.

	updateEntertainerMissionStatus(true, MissionTypes::DANCER);

	entertainer->notifyObservers(ObserverEventType::STARTENTERTAIN, entertainer);

	startEntertaining();
}

void EntertainingSessionImplementation::doPerformEffect(int effectId, int effectLevel) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	if (effectLevel > 3 || effectLevel < 1)
		effectLevel = 3;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();

	if (!entertainer->checkCooldownRecovery("performing_entertainer_effect")) {
		performanceManager->performanceMessageToSelf(entertainer, nullptr, "performance", "effect_wait_self"); // You must wait before you can perform another special effect.
		return;
	}

	PerformEffect* effect = performanceManager->getPerformEffect(effectId, effectLevel);

	if (effect == nullptr) {
		error() << "Unable to get performance effect using id " << effectId << " and level " << effectLevel;
		return;
	}

	if ((isDancing() && !effect->isDanceEffect()) || (isPlayingMusic() && !effect->isMusicEffect())) {
		performanceManager->performanceMessageToSelf(entertainer, nullptr, "performance", "effect_not_performing_correct"); // You are not using the correct performance skill to execute this special effect.
		return;
	}

	int effectCost = effect->getEffectActionCost();
	effectCost = entertainer->calculateCostAdjustment(CreatureAttribute::QUICKNESS, effectCost);

	if (entertainer->getHAM(CreatureAttribute::ACTION) <= effectCost) {
		performanceManager->performanceMessageToSelf(entertainer, nullptr, "performance", "effect_too_tired"); // You are too tired to execute this special effect.
		return;
	}

	int targetType = effect->getTargetType();
	String effectFile = effect->getEffectFile();
	String effectMessage = effect->getEffectMessage();

	if (targetType == PerformEffect::TARGET_SELF) {
		entertainer->playEffect(effectFile, "");
	} else if (targetType == PerformEffect::TARGET_STATIONARY) {
		PlayClientEffectLoc* effectLoc = new PlayClientEffectLoc(effectFile, entertainer->getZone()->getZoneName(), entertainer->getPositionX(), entertainer->getPositionZ(), entertainer->getPositionY(), entertainer->getParentID());
		entertainer->broadcastMessage(effectLoc, true);
	} else if (targetType == PerformEffect::TARGET_OTHER) {
		uint64 targetID = entertainer->getTargetID();
 		ManagedReference<CreatureObject*> targetCreature = entertainer->getZoneServer()->getObject(targetID).castTo<CreatureObject*>();

 		if (targetCreature == nullptr || !targetCreature->isPlayerCreature()) {
 			performanceManager->performanceMessageToSelf(entertainer, nullptr, "performance", "effect_need_target"); // This special effect requires an active target to execute.
 			return;
 		} else {
 			targetCreature->playEffect(effectFile, "");
 		}
	}

	performanceManager->performanceMessageToSelf(entertainer, nullptr, "performance", effectMessage);

	entertainer->inflictDamage(entertainer, CreatureAttribute::ACTION, effectCost, true);

	uint64 effectDuration = (uint64)(effect->getEffectDuration() * 1000);
	entertainer->addCooldown("performing_entertainer_effect", effectDuration);
}

void EntertainingSessionImplementation::startPlayingMusic(int perfIndex, Instrument* instrument) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	if (instrument == nullptr) {
		entertainer->sendSystemMessage("@performance:music_no_instrument"); // You must have an instrument equipped to play music.
		return;
	}

	performanceIndex = perfIndex;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	Locker locker(entertainer);

	if (instrument->getInstrumentType() == Instrument::OMNIBOX || instrument->getInstrumentType() == Instrument::NALARGON) {
		bool isStatic = instrument->getObjectID() < 10000000;
		bool isOwnedByPlayer = instrument->getSpawnerPlayer() == entertainer;

		if (isOwnedByPlayer) {
			instrument->initializePosition(entertainer->getPositionX(), entertainer->getPositionZ(), entertainer->getPositionY());
			instrument->setDirection(*entertainer->getDirection());
		} else if (isStatic) {
			entertainer->setDirection(*instrument->getDirection());
			entertainer->teleport(instrument->getPositionX(), instrument->getPositionZ(), instrument->getPositionY(), instrument->getParentID());
		}
	}

	sendEntertainingUpdate(entertainer, performanceIndex, true);

	entertainer->setListenToID(entertainer->getObjectID(), true);

	updateEntertainerMissionStatus(true, MissionTypes::MUSICIAN);

	entertainer->notifyObservers(ObserverEventType::STARTENTERTAIN, entertainer);

	startEntertaining();
}

void EntertainingSessionImplementation::startEntertaining() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	Locker locker(entertainer);

	startTickTask();

	if (observer == nullptr) {
		observer = new EntertainingObserver();
		observer->deploy();
	}

	entertainer->registerObserver(ObserverEventType::POSTURECHANGED, observer);
}

void EntertainingSessionImplementation::stopDancing() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	bool disableAutoAfterStop = false;
	bool keepSessionActive = false;

	{
		Locker locker(entertainer);

		if (!isDancing())
			return;

		keepSessionActive = autoCycleEnabled && autoCycleStageChanging;

		entertainer->sendSystemMessage("@performance:dance_stop_self"); // You stop dancing.

		performanceIndex = 0;

		entertainer->dropObserver(ObserverEventType::POSTURECHANGED, observer);
		entertainer->setPosture(CreaturePosture::UPRIGHT, true, true);

		ManagedReference<PlayerManager*> playerManager = entertainer->getZoneServer()->getPlayerManager();

		while (patronDataMap.size() > 0) {
			ManagedReference<CreatureObject*> patron = patronDataMap.elementAt(0).getKey();

			Locker clocker(patron, entertainer);

			playerManager->stopWatch(patron, entertainer->getObjectID(), true, true, false);

			if (!patron->isWatching())
				sendEntertainmentUpdate(patron, 0, "");

			patronDataMap.drop(patron);
		}

		if (tickTask != nullptr && tickTask->isScheduled())
			tickTask->cancel();

		entertainer->notifyObservers(ObserverEventType::STOPENTERTAIN, entertainer);

		updateEntertainerMissionStatus(false, MissionTypes::DANCER);
		sendEntertainingUpdate(entertainer, 0, false);

		if (!isDancing() && !isPlayingMusic()) {
			ManagedReference<PlayerObject*> entPlayer = entertainer->getPlayerObject();

			if (entPlayer != nullptr && entPlayer->getPerformanceBuffTarget() != 0)
				entPlayer->setPerformanceBuffTarget(0);

			if (!keepSessionActive)
				entertainer->dropActiveSession(SessionFacadeType::ENTERTAINING);
		}

		if (autoCycleEnabled && !autoCycleStageChanging)
			disableAutoAfterStop = true;
	}

	if (disableAutoAfterStop)
		disableAutoCycle(false);
}

int EntertainingSessionImplementation::handleAutoCycleTick() {
	if (!autoCycleEnabled)
		return ENTERTAINER_AUTO_ACTION_NONE;

	switch (autoCycleStage) {
	case ENTERTAINER_AUTO_STAGE_NONE:
		return ENTERTAINER_AUTO_ACTION_NONE;
	case ENTERTAINER_AUTO_STAGE_DANCE:
		if (!isDancing())
			return autoCycleStageChanging ? ENTERTAINER_AUTO_ACTION_NONE : ENTERTAINER_AUTO_ACTION_DISABLE;
		break;
	case ENTERTAINER_AUTO_STAGE_MUSIC:
		if (!isPlayingMusic())
			return autoCycleStageChanging ? ENTERTAINER_AUTO_ACTION_NONE : ENTERTAINER_AUTO_ACTION_DISABLE;
		break;
	default:
		return ENTERTAINER_AUTO_ACTION_DISABLE;
	}

	// Auto-flourish execution for faster buff building
	if (++autoFlourishTickCounter >= autoFlourishInterval) {
		autoFlourishTickCounter = 0;
		
		ManagedReference<CreatureObject*> ent = entertainer.get();
		if (ent != nullptr && flourishCount < 5) {
			// Execute flourish (use flourish 1, doesn't grant XP during auto-cycle to prevent exploitation)
			doFlourish(1, false);
		}
	}

	if (autoCycleTicksRemaining > 0 && --autoCycleTicksRemaining > 0)
		return ENTERTAINER_AUTO_ACTION_NONE;

	autoCycleStageChanging = true;
	return ENTERTAINER_AUTO_ACTION_ADVANCE_STAGE;
}

void EntertainingSessionImplementation::enableAutoCycle(int danceIndex, int musicIndex, int musicInstrumentType) {
	ManagedReference<CreatureObject*> ent = entertainer.get();

	if (ent == nullptr)
		return;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* dancePerformance = performanceManager->getPerformanceFromIndex(danceIndex);
	Performance* musicPerformance = performanceManager->getPerformanceFromIndex(musicIndex);

	if (dancePerformance == nullptr || !dancePerformance->isDance() || musicPerformance == nullptr || !musicPerformance->isMusic()) {
		ent->sendSystemMessage("Unable to enable auto cycle: invalid performance selection.");
		return;
	}

	bool alreadyPerforming = false;

	{
		Locker locker(ent);

		if (isDancing() || isPlayingMusic()) {
			alreadyPerforming = true;
		} else {
			autoCycleEnabled = true;
			autoCycleStageChanging = false;
			autoCycleStage = ENTERTAINER_AUTO_STAGE_NONE;
			autoCycleTicksRemaining = 0;
			autoDancePerformanceIndex = danceIndex;
			autoMusicPerformanceIndex = musicIndex;
			autoMusicInstrumentType = musicInstrumentType;
		}
	}

	if (alreadyPerforming) {
		ent->sendSystemMessage("You must stop performing before enabling auto cycle.");
		return;
	}

	StringBuffer msg;
	msg << "Entertainer auto cycle enabled: dance '" << dancePerformance->getName() << "', music '" << musicPerformance->getName() << "'.";
	ent->sendSystemMessage(msg.toString());

	startAutoCycleStage(ENTERTAINER_AUTO_STAGE_DANCE);
}

void EntertainingSessionImplementation::disableAutoCycle(bool sendMessage) {
	ManagedReference<CreatureObject*> ent = entertainer.get();

	if (ent == nullptr)
		return;

	bool wasEnabled = false;

	{
		Locker locker(ent);
		wasEnabled = autoCycleEnabled;

		if (autoCycleEnabled) {
			autoCycleEnabled = false;
			autoCycleStageChanging = false;
			autoCycleStage = ENTERTAINER_AUTO_STAGE_NONE;
			autoCycleTicksRemaining = 0;
			autoDancePerformanceIndex = 0;
			autoMusicPerformanceIndex = 0;
			autoMusicInstrumentType = -1;
		}
	}

	if (!sendMessage)
		return;

	if (wasEnabled)
		ent->sendSystemMessage("Entertainer auto cycle disabled.");
	else
		ent->sendSystemMessage("Entertainer auto cycle is not active.");
}

void EntertainingSessionImplementation::startAutoCycleStage(int stage) {
	ManagedReference<CreatureObject*> ent = entertainer.get();

	if (ent == nullptr)
		return;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();

	int performanceIndex = 0;
	int storedInstrumentType = autoMusicInstrumentType;

	{
		Locker locker(ent);

		if (!autoCycleEnabled) {
			autoCycleStage = ENTERTAINER_AUTO_STAGE_NONE;
			autoCycleStageChanging = false;
			return;
		}

		autoCycleStage = stage;

		if (stage == ENTERTAINER_AUTO_STAGE_DANCE) {
			performanceIndex = autoDancePerformanceIndex;
		} else if (stage == ENTERTAINER_AUTO_STAGE_MUSIC) {
			performanceIndex = autoMusicPerformanceIndex;
		} else {
			autoCycleStage = ENTERTAINER_AUTO_STAGE_NONE;
			autoCycleStageChanging = false;
			return;
		}
	}

	if (stage == ENTERTAINER_AUTO_STAGE_DANCE) {
		Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

		if (performance == nullptr || !performance->isDance()) {
			disableAutoCycle(true);
			return;
		}

		int ticks = computeAutoCycleTicks(ent, performance, true);

		if (ticks <= 0)
			ticks = 1;

		{
			Locker locker(ent);

			if (!autoCycleEnabled)
				return;

			autoCycleTicksRemaining = ticks;
			autoCycleStageChanging = false;
			
			// Set flourish interval based on performance loop duration
			// Dance loops are 10s, so tick every 10s means flourish every 5 ticks (50s)
			// Music loops are 5s, so flourish every 10 ticks (50s) - keeps consistent timing
			float loopDuration = performance->getLoopDuration();
			autoFlourishInterval = (int)(50.0f / (loopDuration * 2.0f)); // 50 seconds / (loop * 2s tick)
			if (autoFlourishInterval < 1)
				autoFlourishInterval = 1;
			autoFlourishTickCounter = 0;
		}

		startDancing(performanceIndex);

		StringBuffer msg;
		msg << "Auto cycle: starting dance '" << performance->getName() << "'.";
		ent->sendSystemMessage(msg.toString());

		return;
	}

	Reference<Instrument*> instrument = ent->getPlayableInstrument();

	if (instrument == nullptr) {
		ent->sendSystemMessage("@performance:music_no_instrument");
		disableAutoCycle(true);
		return;
	}

	int currentInstrumentType = instrument->getInstrumentType();
	int resolvedIndex = performanceIndex;

	if (currentInstrumentType != storedInstrumentType) {
		int matchingIndex = performanceManager->getMatchingPerformanceIndex(performanceIndex, currentInstrumentType);

		if (matchingIndex != 0)
			resolvedIndex = matchingIndex;
	}

	Performance* performance = performanceManager->getPerformanceFromIndex(resolvedIndex);

	if (performance == nullptr || !performance->isMusic()) {
		ent->sendSystemMessage("Auto cycle: unable to start configured song.");
		disableAutoCycle(true);
		return;
	}

	if (!performanceManager->canPlayInstrument(ent, currentInstrumentType)) {
		performanceManager->performanceMessageToSelf(ent, nullptr, "performance", "music_lack_skill_instrument");
		disableAutoCycle(true);
		return;
	}

	if (!performanceManager->canPlaySong(ent, resolvedIndex)) {
		performanceManager->performanceMessageToSelf(ent, nullptr, "performance", "music_lack_skill_song_self");
		disableAutoCycle(true);
		return;
	}

	int ticks = computeAutoCycleTicks(ent, performance, false);

	if (ticks <= 0)
		ticks = 1;

	{
		Locker locker(ent);

		if (!autoCycleEnabled)
			return;

		autoMusicPerformanceIndex = resolvedIndex;
		autoMusicInstrumentType = currentInstrumentType;
		autoCycleTicksRemaining = ticks;
		autoCycleStageChanging = false;
		
		// Set flourish interval based on performance loop duration
		float loopDuration = performance->getLoopDuration();
		autoFlourishInterval = (int)(50.0f / (loopDuration * 2.0f)); // 50 seconds / (loop * 2s tick)
		if (autoFlourishInterval < 1)
			autoFlourishInterval = 1;
		autoFlourishTickCounter = 0;
	}

	startPlayingMusic(resolvedIndex, instrument.get());

	StringBuffer msg;
	msg << "Auto cycle: starting music '" << performance->getName() << "'.";
	ent->sendSystemMessage(msg.toString());
}

void EntertainingSessionImplementation::advanceAutoCycleStage() {
	ManagedReference<CreatureObject*> ent = entertainer.get();

	if (ent == nullptr)
		return;

	int stage = ENTERTAINER_AUTO_STAGE_NONE;

	{
		Locker locker(ent);

		if (!autoCycleEnabled) {
			autoCycleStageChanging = false;
			return;
		}

		stage = autoCycleStage;
	}

	if (stage == ENTERTAINER_AUTO_STAGE_DANCE) {
		stopDancing();

		{
			Locker locker(ent);

			if (!autoCycleEnabled) {
				autoCycleStageChanging = false;
				return;
			}
		}

		startAutoCycleStage(ENTERTAINER_AUTO_STAGE_MUSIC);
	} else if (stage == ENTERTAINER_AUTO_STAGE_MUSIC) {
		stopMusic(true);

		{
			Locker locker(ent);

			if (!autoCycleEnabled) {
				autoCycleStageChanging = false;
				return;
			}
		}

		startAutoCycleStage(ENTERTAINER_AUTO_STAGE_DANCE);
	} else {
		disableAutoCycle(false);
		return;
	}

	{
		Locker locker(ent);
		autoCycleStageChanging = false;
	}
}

bool EntertainingSessionImplementation::canHealBattleFatigue() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	return entertainer != nullptr && entertainer->getSkillMod("private_med_battle_fatigue") > 0;
}

bool EntertainingSessionImplementation::canGiveEntertainBuff() {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	return entertainer != nullptr && entertainer->getSkillMod("private_buff_mind") > 0;
}

void EntertainingSessionImplementation::addEntertainerFlourishBuff() {
	if (patronDataMap.size() <= 0)
		return;

	for (int i = 0; i < patronDataMap.size(); ++i) {
		ManagedReference<CreatureObject*> patron = patronDataMap.elementAt(i).getKey();

		try {
			increaseEntertainerBuff(patron);
		} catch (Exception& e) {
			error("Unreported exception caught in EntertainingSessionImplementation::addEntertainerFlourishBuff()");
		}
	}
}

void EntertainingSessionImplementation::doFlourish(int flourishNumber, bool grantXp) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	if (!isDancing() && !isPlayingMusic()) {
		entertainer->sendSystemMessage("@performance:flourish_not_performing");
		return;
	}

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	ManagedReference<Instrument*> instrument = entertainer->getPlayableInstrument();

	float baseActionDrain = performance->getActionPointsPerLoop() - (int)(entertainer->getHAM(CreatureAttribute::QUICKNESS)/35.f);

	if (baseActionDrain < 0)
		baseActionDrain = 0;

	//float baseActionDrain = -40 + (getQuickness() / 37.5);
	float flourishActionDrain = baseActionDrain / 2.0;

	int actionDrain = (int)round((flourishActionDrain * 10 + 0.5) / 10.0); // Round to nearest dec for actual int cost

	if (entertainer->getHAM(CreatureAttribute::ACTION) <= actionDrain) {
		entertainer->sendSystemMessage("@performance:flourish_too_tired");
	} else {
		if (actionDrain > 0)
			entertainer->inflictDamage(entertainer, CreatureAttribute::ACTION, actionDrain, false, true);

		if (isDancing()) {
			StringBuffer msg;
			msg << "skill_action_" << flourishNumber;
			entertainer->doAnimation(msg.toString());
		} else if (isPlayingMusic()) {
			Flourish* flourish = new Flourish(entertainer, flourishNumber);
			entertainer->broadcastMessage(flourish, true);
		}

		//check to see how many flourishes have occurred this tick
		if (flourishCount < 5) {
			// Add buff
			addEntertainerFlourishBuff();

			// Grant Experience
			float loopDuration = performance->getLoopDuration();

			int flourishCap = (int) (10 / loopDuration); // Cap for how many flourishes count towards xp per pulse. Music loops are 5s, dance are 10s so music has a cap of 2, dance a cap of 1.

			if (grantXp && flourishCount < flourishCap)
				flourishXp += performance->getFlourishXpMod();

			flourishCount++;
		}
		entertainer->notifyObservers(ObserverEventType::FLOURISH, entertainer, flourishNumber);

		entertainer->sendSystemMessage("@performance:flourish_perform"); // You perform a flourish.
	}
}

void EntertainingSessionImplementation::addEntertainerBuffDuration(CreatureObject* creature, int performanceType, float duration) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();
	int buffDuration = getEntertainerBuffDuration(creature, performanceType);

	buffDuration += duration;

	// City Specialist skill mod for additional entertainer buff duration
	if (buffDuration > (120.0f + (10.0f / 60.0f) + (float) entertainer->getSkillMod("increase_entertainer_buff"))) // 2 hrs 10 seconds + city spec bonus
		buffDuration = (120.0f + (10.0f / 60.0f) + (float) entertainer->getSkillMod("increase_entertainer_buff")); // 2hrs 10 seconds + city spec bonus

	setEntertainerBuffDuration(creature, performanceType, buffDuration);
}

void EntertainingSessionImplementation::addEntertainerBuffStrength(CreatureObject* creature, int performanceType, float strength) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	int buffStrength = getEntertainerBuffStrength(creature, performanceType);


	float newBuffStrength = buffStrength + strength;

	float maxBuffStrength = 0.0f;	//cap based on enhancement skill

	if (isDancing()) {
		maxBuffStrength = (float) entertainer->getSkillMod("healing_dance_mind");
	} else if (isPlayingMusic()) {
		maxBuffStrength = (float) entertainer->getSkillMod("healing_music_mind");
	}

	if (maxBuffStrength > 125.0f)
		maxBuffStrength = 125.0f;	//cap at 125% power

	float factionPerkStrength = entertainer->getSkillMod("private_faction_buff_mind");

	ManagedReference<BuildingObject*> building = cast<BuildingObject*>(entertainer->getRootParent());

	if (building != nullptr && factionPerkStrength > 0 && building->isPlayerRegisteredWithin(entertainer->getObjectID())) {
		unsigned int buildingFaction = building->getFaction();
		unsigned int entFaction = entertainer->getFaction();

		if (entFaction != 0 && entFaction == buildingFaction && entertainer->getFactionStatus() == FactionStatus::OVERT) {
			maxBuffStrength += factionPerkStrength;
		}
	}

	//add xp based on % added to buff strength
	if (newBuffStrength  < maxBuffStrength) {
		healingXp += strength;
	} else {
		healingXp += maxBuffStrength - buffStrength;
		newBuffStrength = maxBuffStrength;
	}

	setEntertainerBuffStrength(creature, performanceType, newBuffStrength);
}

void EntertainingSessionImplementation::setEntertainerBuffDuration(CreatureObject* creature, int performanceType, float duration) {
	if (!patronDataMap.contains(creature))
		return;

	EntertainingData* data = &patronDataMap.get(creature);

	if (data == nullptr)
		return;

	data->setDuration(duration);
}

int EntertainingSessionImplementation::getEntertainerBuffDuration(CreatureObject* creature, int performanceType) {
	if (!patronDataMap.contains(creature))
		return 0;

	EntertainingData* data = &patronDataMap.get(creature);

	if (data == nullptr)
		return 0;

	return data->getDuration();
}

int EntertainingSessionImplementation::getEntertainerBuffStrength(CreatureObject* creature, int performanceType) {
	if (!patronDataMap.contains(creature))
		return 0;

	EntertainingData* data = &patronDataMap.get(creature);

	if (data == nullptr)
		return 0;

	return data->getStrength();
}

int EntertainingSessionImplementation::getEntertainerBuffStartTime(CreatureObject* creature, int performanceType) {
	if (!patronDataMap.contains(creature))
		return 0;

	EntertainingData* data = &patronDataMap.get(creature);

	if (data == nullptr)
		return 0;

	return data->getTimeStarted();
}

void EntertainingSessionImplementation::setEntertainerBuffStrength(CreatureObject* creature, int performanceType, float strength) {
	if (!patronDataMap.contains(creature))
		return;

	EntertainingData* data = &patronDataMap.get(creature);

	if (data == nullptr)
		return;

	data->setStrength(strength);
}

void EntertainingSessionImplementation::sendEntertainmentUpdate(CreatureObject* creature, uint64 entid, const String& mood) {
	CreatureObject* entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	if (entertainer->isPlayingMusic()) {
		creature->setListenToID(entid, true);
	} else if (entertainer->isDancing()) {
		creature->setWatchToID(entid);
	}

	String moodString = creature->getZoneServer()->getChatManager()->getMoodAnimation(mood);
	creature->setMoodString(moodString, true);
}

void EntertainingSessionImplementation::sendEntertainingUpdate(CreatureObject* creature, int performanceType, bool startPerformance) {
	performanceIndex = performanceType;
	String performanceAnim = "";

	if (performanceType > 0) {
		creature->setPosture(CreaturePosture::SKILLANIMATING);

		PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
		Performance* performance = performanceManager->getPerformanceFromIndex(performanceType);

		if (performance == nullptr)
			return;

		if (performance->isMusic())
			performanceAnim = performanceManager->getInstrumentAnimation(performance->getInstrumentAudioId());
		else
			performanceAnim = performanceManager->getDanceAnimation(performance->getPerformanceIndex());
	}

	creature->setPerformanceAnimation(performanceAnim, startPerformance);
	creature->setPerformanceStartTime(0, startPerformance);
	creature->setPerformanceType(performanceType, true);
}

void EntertainingSessionImplementation::activateEntertainerBuff(CreatureObject* creature, int performanceType) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	try {
		//Check if on Deny Service list
		if (isInDenyServiceList(creature))
			return;

		ManagedReference<PlayerObject*> entPlayer = entertainer->getPlayerObject();
		//Check if the patron is a valid buff target
		//Whether it be passive(in the same group) or active (/setPerform target)
		bool sameGroup = entertainer->isGrouped() && entertainer->getGroupID() == creature->getGroupID();
		bool isTargeted = entPlayer != nullptr && entPlayer->getPerformanceBuffTarget() == creature->getObjectID();
		bool isActivePatron = patronDataMap.contains(creature);

		if (!sameGroup && !isTargeted && !isActivePatron)
			return;

		if (creature->isIncapacitated() || creature->isDead()) {
			return;
		}

		if (!canGiveEntertainBuff())
			return;

		// Returns the Number of Minutes for the Buff Duration
		float buffDuration = getEntertainerBuffDuration(creature, performanceType);

		if (buffDuration * 60 < 10.0f) { //10 sec minimum buff duration
			return;
		}

		//1 minute minimum listen/watch time
		int timeElapsed = time(0) - getEntertainerBuffStartTime(creature, performanceType);
		if (timeElapsed < 60) {
			creature->sendSystemMessage("You must listen or watch a performer for at least 1 minute in order to gain the entertainer buffs.");
			return;
		}

		// Returns a % of base stat
		int campModTemp = 100;


		float buffStrength = getEntertainerBuffStrength(creature, performanceType) / 100.0f;

		if (buffStrength == 0)
			return;

		ManagedReference<PerformanceBuff*> oldBuff = nullptr;
		switch (performanceType) {
		case PerformanceType::MUSIC:
		{
			uint32 focusBuffCRC = STRING_HASHCODE("performance_enhance_music_focus");
			uint32 willBuffCRC = STRING_HASHCODE("performance_enhance_music_willpower");
			oldBuff = cast<PerformanceBuff*>(creature->getBuff(focusBuffCRC));
			if (oldBuff != nullptr && oldBuff->getBuffStrength() > buffStrength)
				return;
			ManagedReference<PerformanceBuff*> focusBuff = new PerformanceBuff(creature, focusBuffCRC, buffStrength, buffDuration * 60, PerformanceBuffType::MUSIC_FOCUS);
			ManagedReference<PerformanceBuff*> willBuff = new PerformanceBuff(creature, willBuffCRC, buffStrength, buffDuration * 60, PerformanceBuffType::MUSIC_WILLPOWER);

			Locker locker(focusBuff);
			creature->addBuff(focusBuff);
			locker.release();

			Locker locker2(willBuff);
			creature->addBuff(willBuff);
			break;
		}
		case PerformanceType::DANCE:
		{
			uint32 mindBuffCRC = STRING_HASHCODE("performance_enhance_dance_mind");
			oldBuff = cast<PerformanceBuff*>(creature->getBuff(mindBuffCRC));
			if (oldBuff != nullptr && oldBuff->getBuffStrength() > buffStrength)
				return;
			ManagedReference<PerformanceBuff*> mindBuff = new PerformanceBuff(creature, mindBuffCRC, buffStrength, buffDuration * 60, PerformanceBuffType::DANCE_MIND);

			Locker locker(mindBuff);
			creature->addBuff(mindBuff);
			break;
		}
		}


	} catch(Exception& e) {

	}

}

void EntertainingSessionImplementation::updateEntertainerMissionStatus(bool entertaining, const int missionType) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	if (entertainer == nullptr)
		return;

	SceneObject* datapad = entertainer->getSlottedObject("datapad");

	if (datapad == nullptr)
		return;

	//Notify all missions of correct type.
	int datapadSize = datapad->getContainerObjectsSize();

	for (int i = 0; i < datapadSize; ++i) {
		if (datapad->getContainerObject(i)->isMissionObject()) {
			Reference<MissionObject*> datapadMission = datapad->getContainerObject(i).castTo<MissionObject*>();

			if (datapadMission != nullptr) {
				EntertainerMissionObjective* objective = cast<EntertainerMissionObjective*>(datapadMission->getMissionObjective());

				if (objective != nullptr && datapadMission->getTypeCRC() == MissionTypes::DANCER && missionType == MissionTypes::DANCER) {
					objective->setIsEntertaining(entertaining);
				} else if (objective != nullptr && datapadMission->getTypeCRC() == MissionTypes::MUSICIAN && missionType == MissionTypes::MUSICIAN) {
					objective->setIsEntertaining(entertaining);
				}
			}
		}
	}
}

void EntertainingSessionImplementation::increaseEntertainerBuff(CreatureObject* patron) {
	ManagedReference<CreatureObject*> entertainer = this->entertainer.get();

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	ManagedReference<Instrument*> instrument = entertainer->getPlayableInstrument();

	if (performanceIndex == 0)
		return;

	if (!isDancing() && (!isPlayingMusic() || !instrument)) {
		cancelSession();
		return;
	}

	if (!canGiveEntertainBuff())
		return;

	ManagedReference<PlayerObject*> entPlayer = entertainer->getPlayerObject();
	//Check if the patron is a valid buff target
	//Whether it be passive(in the same group) or active (/setPerform target)
	bool sameGroup = entertainer->isGrouped() && entertainer->getGroupID() == patron->getGroupID();
	bool isTargeted = entPlayer != nullptr && entPlayer->getPerformanceBuffTarget() == patron->getObjectID();
	bool isActivePatron = patronDataMap.contains(patron);

	if (!sameGroup && !isTargeted && !isActivePatron)
		return;

	if (isInDenyServiceList(patron))
		return;

	float buffAcceleration = 1 + ((float)entertainer->getSkillMod("accelerate_entertainer_buff") / 100.f);

	addEntertainerBuffDuration(patron, performance->getType(), 2.0f * buffAcceleration);
	addEntertainerBuffStrength(patron, performance->getType(), performance->getHealShockWound());

}

void EntertainingSessionImplementation::awardEntertainerExperience() {
	ManagedReference<CreatureObject*> player = this->entertainer.get();
	ManagedReference<PlayerManager*> playerManager = player->getZoneServer()->getPlayerManager();

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return;

	ManagedReference<Instrument*> instrument = player->getPlayableInstrument();

	if (player->isPlayerCreature()) {
		if (oldFlourishXp > flourishXp && (isDancing() || isPlayingMusic())) {
			flourishXp = oldFlourishXp;

			if (flourishXp > 0) {
				int flourishDec = (int)((float)performance->getFlourishXpMod() / 6.0f);
				flourishXp -= std::max(1, flourishDec);
			}

			if (flourishXp < 0)
				flourishXp = 0;
		}

		if (flourishXp > 0 && (isDancing() || isPlayingMusic())) {
			String xptype;

			if (isDancing())
				xptype = "dance";
			else if (isPlayingMusic())
				xptype = "music";

			int groupBonusCount = 0;

			ManagedReference<GroupObject*> group = player->getGroup();

			if (group != nullptr) {
				for (int i = 0; i < group->getGroupSize(); ++i) {
					try {
						ManagedReference<CreatureObject *> groupMember = group->getGroupMember(i);

						if (groupMember != nullptr && groupMember->isPlayerCreature()) {
							Locker clocker(groupMember, player);

							if (groupMember != player && groupMember->isEntertaining() &&
								groupMember->isInRange(player, 40.0f) &&
								groupMember->hasSkill("social_entertainer_novice")) {
								++groupBonusCount;
							}
						}
					} catch (ArrayIndexOutOfBoundsException &exc) {
						warning("EntertainingSessionImplementation::awardEntertainerExperience " + exc.getMessage());
					}
				}
			}

			int xpAmount = flourishXp + performance->getBaseXp();

			int audienceSize = std::min(getBandAudienceSize(), 50);
			float audienceMod = audienceSize / 50.f;
			float applauseMod = applauseCount / 100.f;

			float groupMod = groupBonusCount * 0.05;

			float totalBonus = 1.f + groupMod + audienceMod + applauseMod;

			xpAmount = ceil(xpAmount * totalBonus);

			if (playerManager != nullptr)
				playerManager->awardExperience(player, xptype, xpAmount, true);

			oldFlourishXp = flourishXp;
			flourishXp = 0;
		} else {
			oldFlourishXp = 0;
		}

		if (healingXp > 0) {
			String healxptype("entertainer_healing");

			if (playerManager != nullptr)
				playerManager->awardExperience(player, healxptype, healingXp, true);

			healingXp = 0;
		}
	}

	applauseCount = 0;
	healingXp = 0;
	flourishCount = 0;
}

SortedVector<ManagedReference<CreatureObject*> > EntertainingSessionImplementation::getPatrons() {
	SortedVector<ManagedReference<CreatureObject*> > patrons;

	for (int i = 0; i < patronDataMap.size(); i++) {
		ManagedReference<CreatureObject*> patron = patronDataMap.elementAt(i).getKey();

		if (patron != nullptr)
			patrons.add(patron);
	}

	return patrons;
}

int EntertainingSessionImplementation::getBandAudienceSize() {
	ManagedReference<CreatureObject*> player = entertainer.get();

	if (player == nullptr)
		return 0;

	int bandAudienceSize = getAudienceSize();

	ManagedReference<GroupObject *> group = player->getGroup();

	if (group == nullptr)
		return bandAudienceSize;

	for (int i = 0; i < group->getGroupSize(); ++i) {
		ManagedReference<CreatureObject *> groupMember = group->getGroupMember(i);

		if (groupMember == nullptr || !groupMember->isPlayerCreature() || groupMember == player)
			continue;

		Locker clocker(groupMember, player);

		ManagedReference<EntertainingSession *> session = groupMember->getActiveSession(SessionFacadeType::ENTERTAINING).castTo<EntertainingSession *>();

		if (session == nullptr)
			continue;

		if (!session->isDancing() && !session->isPlayingMusic())
			continue;

		bandAudienceSize += session->getAudienceSize();
	}

	return bandAudienceSize;
}

String EntertainingSessionImplementation::getPerformanceName() {
	if (performanceIndex == 0)
		return "";

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return "";

	return performance->getName();
}

void EntertainingSessionImplementation::addPatron(CreatureObject* patron) {
	if (patronDataMap.contains(patron))
		patronDataMap.drop(patron);

	EntertainingData data;

	patronDataMap.put(patron, data);
}


void EntertainingSessionImplementation::removePatron(CreatureObject* patron) {
	if (!patronDataMap.contains(patron))
		return;

	patronDataMap.drop(patron);
}

bool EntertainingSessionImplementation::isPlayingMusic() {
	if (performanceIndex == 0)
		return false;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return false;

	return performance->isMusic();
}

bool EntertainingSessionImplementation::isDancing() {
	if (performanceIndex == 0)
		return false;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
	Performance* performance = performanceManager->getPerformanceFromIndex(performanceIndex);

	if (performance == nullptr)
		return false;

	return performance->isDance();
}

void EntertainingSessionImplementation::joinBand() {
	if (performanceIndex == 0)
		return;

	ManagedReference<CreatureObject*> player = entertainer.get();

	if (player == nullptr)
		return;

	ManagedReference<GroupObject*> group = player->getGroup();

	if (group == nullptr)
		return;

	Reference<Instrument*> instrument = player->getPlayableInstrument();

	if (instrument == nullptr)
		return;

	int instrumentType = instrument->getInstrumentType();
	bool activeBandSong = false;
	int bandSong = 0;

	PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();

	for (int i = 0; i < group->getGroupSize(); i++) {
		ManagedReference<CreatureObject*> groupMember = group->getGroupMember(i);

		if (groupMember == nullptr || groupMember == player || !groupMember->isPlayingMusic())
			continue;

		ManagedReference<EntertainingSession*> memberSession = groupMember->getActiveSession(SessionFacadeType::ENTERTAINING).castTo<EntertainingSession*>();

		if (memberSession == nullptr)
			continue;

		int memberPerformanceIndex = memberSession->getPerformanceIndex();

		if (memberPerformanceIndex == 0)
			continue;

		bandSong = performanceManager->getMatchingPerformanceIndex(memberPerformanceIndex, instrumentType);
		activeBandSong = true;
		break;
	}

	if (activeBandSong && performanceIndex != bandSong) {
		performanceManager->performanceMessageToSelf(player, nullptr, "performance", "music_join_band_stop"); // You must play the same song as the band.
		stopMusic(true);
	}

}
