#ifndef PLUGININTERFACEFORWARD_H_
#define PLUGININTERFACEFORWARD_H_

#include <cstdint>

// Forward declarations to avoid including heavy headers in plugins
namespace server {
namespace zone {
	namespace objects {
		namespace creature {
			class CreatureObject;
		}
	}
namespace managers {
namespace plugin {

// Forward declare the actual interfaces
class IPlugin;
class IEventListener;
class ICommandHandler;

// Lightweight wrapper for plugin data structures
struct ChatEventDataLight {
	void* sender;
	const char* senderName;
	uint64_t senderOID;
	uint32_t senderAccountID;
	const char* message;
	const char* channelType;
	const char* recipientName;
	uint64_t recipientOID;
	const char* planet;
	float posX, posY, posZ;
	uint64_t timestamp;
};

struct PlayerEventDataLight {
	void* player;
	const char* playerName;
	uint64_t playerOID;
	uint32_t accountID;
	const char* eventType;
	const char* details;
	const char* planet;
	float posX, posY, posZ;
	uint64_t timestamp;
};

struct CommandEventDataLight {
	void* executor;
	const char* executorName;
	uint64_t executorOID;
	uint32_t executorAccountID;
	const char* command;
	const char* arguments;
	void* target;
	const char* targetName;
	uint64_t targetOID;
	const char* result;
	bool success;
	uint64_t timestamp;
};

struct CommandParametersLight {
	const char* command;
	void* executor;
	void* target;
	void* params; // Actually a HashTable but plugins don't need to know
};

// Plugin factory type
typedef IPlugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(IPlugin*);

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

#endif // PLUGININTERFACEFORWARD_H_