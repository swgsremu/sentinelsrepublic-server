/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef OPENCLINICCOMMAND_H_
#define OPENCLINICCOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/scene/SessionFacadeType.h"
#include "server/zone/managers/skill/SkillModManager.h"
#include "server/zone/objects/player/sessions/MedicalServiceSession.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "system/io/StringTokenizer.h"

class OpenClinicCommand : public QueueCommand {
public:
	OpenClinicCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		if (!hasRequiredSkill(creature)) {
			creature->sendSystemMessage("You must be a Master Medic or Master Doctor to open clinic services.");
			return GENERALERROR;
		}

		if (creature->isInCombat()) {
			creature->sendSystemMessage("You cannot open clinic services while in combat.");
			return GENERALERROR;
		}

		if (!hasMedicalSupport(creature)) {
			creature->sendSystemMessage("You must be inside a medical facility or have a medical droid available.");
			return GENERALERROR;
		}

		int woundPrice = 0;
		bool hasWoundPrice = false;

		StringTokenizer tokenizer(arguments.toString());
		tokenizer.setDelimeter(" ");

		if (tokenizer.hasMoreTokens()) {
			woundPrice = tokenizer.getIntToken();
			hasWoundPrice = true;
		}

		ManagedReference<MedicalServiceSession*> session = creature->getActiveSession(SessionFacadeType::MEDICALSERVICE).castTo<MedicalServiceSession*>();

		if (session != nullptr) {
			Locker locker(session);

			session->updateMode(false);
			session->updateIncludeResists(false);

			if (hasWoundPrice)
				session->updatePrices(session->getEnhancementPrice(), woundPrice);

			if (!session->isActive()) {
				session->openSession(session->getEnhancementPrice(), woundPrice, session->includesResists());
			} else if (!hasWoundPrice) {
				session->closeSession(true);
				creature->dropActiveSession(SessionFacadeType::MEDICALSERVICE);
			} else {
				creature->sendSystemMessage("Clinic price updated.");
			}

			return SUCCESS;
		}

		session = new MedicalServiceSession(creature, false);
		session->updateIncludeResists(false);
		creature->addActiveSession(SessionFacadeType::MEDICALSERVICE, session);

		session->openSession(0, woundPrice, false);

		return SUCCESS;
	}

private:
	bool hasRequiredSkill(CreatureObject* creature) const {
		return creature->hasSkill("science_medic_master") || creature->hasSkill("science_doctor_master");
	}

	bool hasMedicalSupport(CreatureObject* creature) const {
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
};

#endif /* OPENCLINICCOMMAND_H_ */
