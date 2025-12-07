/*
			Copyright <SWGEmu>
	See file COPYING for copying conditions.*/

#ifndef OPENHOSPITALCOMMAND_H_
#define OPENHOSPITALCOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/scene/SessionFacadeType.h"
#include "server/zone/managers/skill/SkillModManager.h"
#include "server/zone/objects/player/sessions/MedicalServiceSession.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "system/io/StringTokenizer.h"

namespace {
	inline bool hasDoctorSkill(CreatureObject* creature) {
		return creature->hasSkill("science_doctor_master");
	}

	inline bool hasMedicalSupport(CreatureObject* creature) {
		int structureMedicalRating = creature->getSkillModOfType("private_medical_rating", SkillModManager::STRUCTURE);
		int droidMedicalRating = creature->getSkillModOfType("private_medical_rating", SkillModManager::DROID);
		int personalMedicalRating = creature->getSkillMod("private_medical_rating");
		int cityBonus = creature->getSkillModOfType("private_medical_rating", SkillModManager::CITY);

		int effectiveRating = personalMedicalRating - cityBonus;

		Reference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost != nullptr && ghost->hasRemoteMedicalSupport() && ghost->getRemoteMedicalRating() > 0)
			return true;

		if (effectiveRating <= 0 || (structureMedicalRating <= 0 && droidMedicalRating <= 0))
			return false;

		return true;
	}
}

class OpenHospitalCommand : public QueueCommand {
public:
	OpenHospitalCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		if (!hasDoctorSkill(creature)) {
			creature->sendSystemMessage("You must be a Master Doctor to open hospital services.");
			return GENERALERROR;
		}

		if (creature->isInCombat()) {
			creature->sendSystemMessage("You cannot open hospital services while in combat.");
			return GENERALERROR;
		}

		if (!hasMedicalSupport(creature)) {
			creature->sendSystemMessage("You must be inside a medical facility or have a medical droid available.");
			return GENERALERROR;
		}

		int enhancementPrice = 0;
		bool hasEnhancementPrice = false;
		int woundPrice = 0;
		bool hasWoundPrice = false;

		StringTokenizer tokenizer(arguments.toString());
		tokenizer.setDelimeter(" ");

		if (tokenizer.hasMoreTokens()) {
			enhancementPrice = tokenizer.getIntToken();
			hasEnhancementPrice = true;
		}

		if (tokenizer.hasMoreTokens()) {
			woundPrice = tokenizer.getIntToken();
			hasWoundPrice = true;
		}

		ManagedReference<MedicalServiceSession*> session = creature->getActiveSession(SessionFacadeType::MEDICALSERVICE).castTo<MedicalServiceSession*>();

		if (session != nullptr) {
			Locker locker(session);

			session->updateMode(true);
			session->updateIncludeResists(true);

			if (hasEnhancementPrice || hasWoundPrice)
				session->updatePrices(hasEnhancementPrice ? enhancementPrice : session->getEnhancementPrice(),
						hasWoundPrice ? woundPrice : session->getWoundPrice());

			if (!session->isActive()) {
				session->openSession(enhancementPrice, woundPrice, true);
			} else if (!hasEnhancementPrice && !hasWoundPrice) {
				session->closeSession(true);
				creature->dropActiveSession(SessionFacadeType::MEDICALSERVICE);
			} else {
				creature->sendSystemMessage("Hospital prices updated.");
			}

			return SUCCESS;
		}

		session = new MedicalServiceSession(creature, true);

		creature->addActiveSession(SessionFacadeType::MEDICALSERVICE, session);

		session->openSession(enhancementPrice, woundPrice, true);

		return SUCCESS;
	}
};

#endif /* OPENHOSPITALCOMMAND_H_ */
