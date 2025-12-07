/*
 * Copyright (c) 2024 - on Wasted Potential Studios.
 * Proprietary code and work not for distribution.
 */

#ifndef AUTOENTERTAINCOMMAND_H_
#define AUTOENTERTAINCOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/player/sessions/EntertainingSession.h"
#include "server/zone/objects/tangible/Instrument.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/managers/skill/PerformanceManager.h"

class AutoEntertainCommand : public QueueCommand {
public:
	AutoEntertainCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		String args = arguments.toString().trim();

		if (args.isEmpty()) {
			creature->sendSystemMessage("Usage: /autoentertain <dance> <song>");
			return GENERALERROR;
		}

		StringTokenizer tokenizer(args);
		tokenizer.setDelimeter(" ");

		String danceName;
		if (!tokenizer.hasMoreTokens()) {
			creature->sendSystemMessage("Usage: /autoentertain <dance> <song>");
			return GENERALERROR;
		}

		tokenizer.getStringToken(danceName);

		StringBuffer songNameBuilder;
		while (tokenizer.hasMoreTokens()) {
			String token;
			tokenizer.getStringToken(token);

			if (songNameBuilder.length() > 0)
				songNameBuilder.append(" ");

			songNameBuilder.append(token);
		}

		String songName = songNameBuilder.toString().trim();

		if (songName.isEmpty()) {
			creature->sendSystemMessage("Usage: /autoentertain <dance> <song>");
			return GENERALERROR;
		}

		PerformanceManager* performanceManager = SkillManager::instance()->getPerformanceManager();
		if (performanceManager == nullptr)
			return GENERALERROR;

		int dancePerformanceIndex = performanceManager->getPerformanceIndex(PerformanceType::DANCE, danceName, 0);
		if (dancePerformanceIndex == 0) {
			creature->sendSystemMessage("@performance:dance_unknown_self");
			return GENERALERROR;
		}

		if (!performanceManager->canPerformDance(creature, dancePerformanceIndex)) {
			creature->sendSystemMessage("@performance:dance_lack_skill_self");
			return GENERALERROR;
		}

		Reference<Instrument*> instrument = creature->getPlayableInstrument();
		if (instrument == nullptr) {
			creature->sendSystemMessage("@performance:music_no_instrument");
			return GENERALERROR;
		}

		int instrumentType = instrument->getInstrumentType();

		if (!performanceManager->canPlayInstrument(creature, instrumentType)) {
			creature->sendSystemMessage("@performance:music_lack_skill_instrument");
			return GENERALERROR;
		}

		int musicPerformanceIndex = performanceManager->getPerformanceIndex(PerformanceType::MUSIC, songName, instrumentType);
		if (musicPerformanceIndex == 0) {
			creature->sendSystemMessage("@performance:music_invalid_song");
			return GENERALERROR;
		}

		if (!performanceManager->canPlaySong(creature, musicPerformanceIndex)) {
			creature->sendSystemMessage("@performance:music_lack_skill_song_self");
			return GENERALERROR;
		}

		ManagedReference<Facade*> facade = creature->getActiveSession(SessionFacadeType::ENTERTAINING);
		ManagedReference<EntertainingSession*> session = dynamic_cast<EntertainingSession*>(facade.get());

		if (session != nullptr && (session->isPlayingMusic() || session->isDancing())) {
			creature->sendSystemMessage("@performance:already_performing_self");
			return GENERALERROR;
		}

		if (session == nullptr) {
			session = new EntertainingSession(creature);
			creature->addActiveSession(SessionFacadeType::ENTERTAINING, session);
		}

		session->enableAutoCycle(dancePerformanceIndex, musicPerformanceIndex, instrumentType);

		return SUCCESS;
	}
};

#endif /* AUTOENTERTAINCOMMAND_H_ */

