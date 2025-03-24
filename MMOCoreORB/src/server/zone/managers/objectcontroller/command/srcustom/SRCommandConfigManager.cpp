#include "../CommandConfigManager.h"
#include "server/zone/srcustom/objects/creatures/commands/PackupStructureCommand.h"

using namespace server::zone::managers::objectcontroller::command;

/**
 * @brief Registers custom SR commands.
 *
 * This method is intended to register any custom commands specific
 * to the Sentinels Republic. It should be implemented to add the
 * custom commands to the command processing system.
 */
void CommandConfigManager::registerSrCommands() {
	commandFactory.registerCommand<PackupStructureCommand>(String("packupstructure").toLowerCase());
}