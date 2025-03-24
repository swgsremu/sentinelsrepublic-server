#ifndef PACKUPSTRUCTURECOMMAND_H_
#define PACKUPSTRUCTURECOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/terminal/Terminal.h"

/**
 * @brief Command class for packing up a structure.
 *
 * This class extends the QueueCommand and provides functionality to pack up a structure.
 */
class PackupStructureCommand final : public QueueCommand {
public:
	/**
	 * @brief Constructor for the PackupStructureCommand class.
	 *
	 * @param name The name of the command.
	 * @param server Pointer to the ZoneProcessServer instance.
	 */
	PackupStructureCommand(const String& name, ZoneProcessServer* server);

	/**
	 * @brief Executes the PackupStructureCommand.
	 *
	 * @param creature Pointer to the CreatureObject executing the command.
	 * @param target The target object ID.
	 * @param arguments Additional arguments for the command.
	 * @return int Status code indicating success or failure.
	 */
	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const override;

private:
/**
 * @brief Message indicating the player must be the owner to pack up the structure.
 */
	static const std::string PACKUP_MUST_BE_OWNER;

	/**
	 * @brief Message indicating the structure is not eligible for pack up.
	 */
	static const std::string PACKUP_NOT_ELIGIBLE;

	/**
	 * @brief Message indicating the packup process is pending.
	 */
	static const std::string PACKUP_PENDING;

	/**
	 * @brief Message indicating the player structure.
	 */
	static const std::string PLAYER_STRUCTURE;
};

#endif // PACKUPSTRUCTURECOMMAND_H_