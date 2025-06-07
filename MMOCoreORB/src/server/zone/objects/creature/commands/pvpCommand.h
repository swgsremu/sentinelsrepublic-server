/*
    Copyright <SWGEmu>
    See file COPYING for copying conditions.
*/

#ifndef PVPCOMMAND_H_
#define PVPCOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"

// Command handler for opening the PVP command window via Lua screenplay.
class pvpCommand : public QueueCommand {
public:

    pvpCommand(const String& name, ZoneProcessServer* server)
        : QueueCommand(name, server) {}

    int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const override {
        if (!creature || !checkStateMask(creature))
            return INVALIDSTATE;

        if (!checkInvalidLocomotions(creature))
            return INVALIDLOCOMOTION;

        Lua* lua = DirectorManager::instance()->getLuaInstance();
        if (!lua)
            return GENERALERROR;

        Reference<LuaFunction*> pvpCommandScreenplay = lua->createFunction("pvpCommandScreenplay", "openInitialWindow", 0);
        if (!pvpCommandScreenplay)
            return GENERALERROR;

        *pvpCommandScreenplay << creature;
        pvpCommandScreenplay->callFunction();

        return SUCCESS;
    }
};

#endif // PVPCOMMAND_H_
