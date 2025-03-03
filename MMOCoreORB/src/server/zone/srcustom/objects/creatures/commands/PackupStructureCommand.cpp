
#include "PackupStructureCommand.h"

#include <server/zone/srcustom/objects/structure/SRStructureObject.h>

#include "server/zone/srcustom/managers/configuration/SRConfigManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/srcustom/objects/scene/SRSessionFacadeType.h"


const std::string PackupStructureCommand::PACKUP_MUST_BE_OWNER = "@player_structure:packup_must_be_owner";
const std::string PackupStructureCommand::PACKUP_NOT_ELIGIBLE = "@player_structure:packup_not_eligible_01";

/**
 * @brief Constructor for the PackupStructureCommand class.
 *
 * @param name The name of the command.
 * @param server Pointer to the ZoneProcessServer instance.
 */
PackupStructureCommand::PackupStructureCommand(const String& name, ZoneProcessServer* server)
	: QueueCommand(name, server) {
}



/**
 * @brief Executes the PackupStructureCommand.
 *
 * @param creature Pointer to the CreatureObject executing the command.
 * @param target The target object ID.
 * @param arguments Additional arguments for the command.
 * @return int Status code indicating success or failure.
 */
int PackupStructureCommand::doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
	if (!checkStateMask(creature))
		return INVALIDSTATE;

	if (!checkInvalidLocomotions(creature))
		return INVALIDLOCOMOTION;

	if (creature->containsActiveSession(SRSessionFacadeType::PACKUPSTRUCTURE)) {
		creature->sendSystemMessage("@player_structure:pending_packup");
		return GENERALERROR;
	}

	const ManagedReference<PlayerManager*> playerManager = server->getPlayerManager();

	const uint64 target_id = creature->getTargetID();
	const ManagedReference<SceneObject*> obj = playerManager->getInRangeStructureWithAdminRights(creature, target_id);

	if (obj == nullptr || !obj->isStructureObject())
		return INVALIDTARGET;

	auto* structure = cast<StructureObject*>( obj.get());

	const ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost == nullptr)
		return GENERALERROR;

	if (!ghost->isOwnedStructure(structure) && !ghost->isStaff()) {
		creature->sendSystemMessage(PACKUP_MUST_BE_OWNER);
		return INVALIDTARGET;
	}

	if (!SRConfigManager::instance()->getStructurePackupEnabled()) {
		creature->sendSystemMessage(PACKUP_NOT_ELIGIBLE);
		return INVALIDTARGET;
	}

	auto message = structure->getSrStructureObject()->getPackupMessage();
	// if (!message.isEmpty()) {
	// 	creature->sendSystemMessage("@player_structure:" + message);
	// 	return INVALIDTARGET;
	// }

	// ManagedReference<PackupStructureSession*> session = new PackupStructureSession(creature, structure);
	// session->initializeSession();

	return SUCCESS;
}