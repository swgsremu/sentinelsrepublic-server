#ifndef PACKUPSTRUCTURECOMMAND_H_
#define PACKUPSTRUCTURECOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/terminal/Terminal.h"

class PackupStructureCommand : public QueueCommand {
public:
	PackupStructureCommand(const String& name, ZoneProcessServer* server);

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const override;
};

#endif // PACKUPSTRUCTURECOMMAND_H_