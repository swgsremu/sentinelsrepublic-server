#ifndef PLUGININTERFACE_H_
#define PLUGININTERFACE_H_

#include "engine/engine.h"
#include "server/zone/objects/creature/CreatureObject.h"

namespace server {
namespace zone {
namespace managers {
namespace plugin {

/**
 * Generic event data structures for plugin system
 */
struct ChatEventData {
	Reference<CreatureObject*> sender;
	String senderName;
	uint64 senderOID;
	uint32 senderAccountID;
	String message;
	String channelType;
	String recipientName;
	uint64 recipientOID;
	String planet;
	float posX, posY, posZ;
	Time timestamp;
	
	ChatEventData() : senderOID(0), senderAccountID(0), recipientOID(0), 
		posX(0), posY(0), posZ(0) {
		timestamp.updateToCurrentTime();
	}
};

struct PlayerEventData {
	Reference<CreatureObject*> player;
	String playerName;
	uint64 playerOID;
	uint32 accountID;
	String eventType; // login, logout, trade, mail, structure_placed, etc.
	HashTable<String, String> parameters;
	Time timestamp;
	
	PlayerEventData() : playerOID(0), accountID(0) {
		timestamp.updateToCurrentTime();
	}
};

struct CommandEventData {
	Reference<CreatureObject*> executor;
	String executorName;
	uint64 executorOID;
	uint32 executorAccountID;
	String command;
	String arguments;
	Reference<CreatureObject*> target;
	String targetName;
	uint64 targetOID;
	String result;
	bool success;
	Time timestamp;
	
	CommandEventData() : executorOID(0), executorAccountID(0), targetOID(0), success(false) {
		timestamp.updateToCurrentTime();
	}
};

struct SystemEventData {
	String eventType;
	String message;
	HashTable<String, String> parameters;
	Time timestamp;
	
	SystemEventData() {
		timestamp.updateToCurrentTime();
	}
};

/**
 * Command parameters for plugin command handling
 */
struct CommandParameters {
	String command;
	Reference<CreatureObject*> executor;
	Reference<CreatureObject*> target;
	HashTable<String, String> params;
	
	CommandParameters() {}
};

/**
 * Interface for event listeners
 * Plugins implement this to receive game events
 */
class IEventListener : public virtual Object {
public:
	virtual ~IEventListener() = default;
	
	// Event callbacks - plugins override what they need
	virtual void onChatEvent(const ChatEventData& data) {}
	virtual void onPlayerEvent(const PlayerEventData& data) {}
	virtual void onCommandEvent(const CommandEventData& data) {}
	virtual void onSystemEvent(const SystemEventData& data) {}
	
	// Plugin lifecycle
	virtual void onPluginLoaded() {}
	virtual void onPluginUnloaded() {}
	
	// Plugin information
	virtual String getPluginName() const = 0;
	virtual String getPluginVersion() const = 0;
	virtual String getPluginDescription() const { return ""; }
};

/**
 * Interface for command handlers
 * Plugins implement this to handle custom commands
 */
class ICommandHandler : public virtual Object {
public:
	virtual ~ICommandHandler() = default;
	
	/**
	 * Execute a command
	 * @return true if command was handled, false otherwise
	 */
	virtual bool handleCommand(const String& cmd, const CommandParameters& params) = 0;
	
	/**
	 * Check if this handler can process the given command
	 */
	virtual bool canHandleCommand(const String& cmd) const = 0;
	
	/**
	 * Get list of commands this handler supports
	 */
	virtual Vector<String> getSupportedCommands() const = 0;
	
	/**
	 * Get required admin level for commands
	 */
	virtual int getRequiredAdminLevel(const String& cmd) const { return 0; }
};

/**
 * Plugin base class combining both interfaces
 */
class IPlugin : public IEventListener, public ICommandHandler {
public:
	virtual ~IPlugin() = default;
};

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

using namespace server::zone::managers::plugin;

#endif // PLUGININTERFACE_H_