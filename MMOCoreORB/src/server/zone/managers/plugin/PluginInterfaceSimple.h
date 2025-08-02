#ifndef PLUGININTERFACESIMPLE_H_
#define PLUGININTERFACESIMPLE_H_

#include "PluginInterfaceForward.h"
#include <vector>
#include <string>

namespace server {
namespace zone {
namespace managers {
namespace plugin {

/**
 * Simple plugin interface that doesn't depend on engine types
 * Plugins should implement these interfaces
 */
class IPlugin {
public:
	virtual ~IPlugin() = default;
	
	virtual std::string getPluginName() const = 0;
	virtual std::string getPluginVersion() const = 0;
	virtual void onPluginLoaded() = 0;
	virtual void onPluginUnloaded() = 0;
};

class IEventListener : public virtual IPlugin {
public:
	virtual ~IEventListener() = default;
	
	// Event callbacks - plugins override what they need
	virtual void onChatEvent(const ChatEventDataLight& data) {}
	virtual void onPlayerEvent(const PlayerEventDataLight& data) {}
	virtual void onCommandEvent(const CommandEventDataLight& data) {}
	virtual void onSystemEvent(const void* data) {} // Reserved for future use
};

class ICommandHandler : public virtual IPlugin {
public:
	virtual ~ICommandHandler() = default;
	
	virtual bool handleCommand(const std::string& cmd, const CommandParametersLight& params) = 0;
	virtual bool canHandleCommand(const std::string& cmd) const = 0;
	virtual std::vector<std::string> getSupportedCommands() const = 0;
	virtual int getRequiredAdminLevel(const std::string& cmd) const = 0;
};

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

// Export functions that plugins must implement
extern "C" {
	server::zone::managers::plugin::IPlugin* createPlugin();
	void destroyPlugin(server::zone::managers::plugin::IPlugin* plugin);
}

#endif // PLUGININTERFACESIMPLE_H_