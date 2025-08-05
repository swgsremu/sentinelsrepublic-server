/*
    Copyright <SWGEmu>
    See file COPYING for copying conditions.
*/

#ifndef ECTASKSCOMMAND_H_
#define ECTASKSCOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"

// Command handler for opening the ectasks command window via Lua screenplay.
class EcTasksCommand : public QueueCommand {
public:

	EcTasksCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const override {
		if (!creature || !checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (!lua)
			return GENERALERROR;

		Reference<LuaFunction*> ecTasksScreenplay = lua->createFunction("ecTasksScreenplay", "openInitialWindow", 0);
		if (!ecTasksScreenplay)
            return GENERALERROR;

		*ecTasksScreenplay << creature;
		ecTasksScreenplay->callFunction();

		return SUCCESS;
	}

};

#endif //ECTASKSCOMMAND_H_
