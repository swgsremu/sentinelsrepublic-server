/*
 * Copyright (c) 2024 - on Wasted Potential Studios.
 * Proprietary code and work not for distribution.
 */

#ifndef AUTOGROUPINVITECOMMAND_H_
#define AUTOGROUPINVITECOMMAND_H_

#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/PlayerBitmasks.h"
#include "server/zone/objects/group/GroupObject.h"

class AutoGroupInviteCommand : public QueueCommand {
public:

	AutoGroupInviteCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (creature == nullptr || !creature->isPlayerCreature())
			return GENERALERROR;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		ManagedReference<GroupObject*> group = creature->getGroup();

		if (!ghost->isAutoGroupInviteEnabled() && group != nullptr && group->getLeader() != creature) {
			creature->sendSystemMessage("@group:must_be_leader");
			return GENERALERROR;
		}

		ghost->toggleCharacterBit(PlayerBitmasks::AUTO_GROUP_INVITE);

		if (ghost->isAutoGroupInviteEnabled()) {
			creature->sendSystemMessage("Group auto join enabled.");
		} else {
			creature->sendSystemMessage("Group auto join disabled.");
		}

		return SUCCESS;
	}
};

#endif /* AUTOGROUPINVITECOMMAND_H_ */

