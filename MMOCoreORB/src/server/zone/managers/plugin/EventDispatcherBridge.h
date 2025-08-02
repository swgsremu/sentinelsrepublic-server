#ifndef EVENTDISPATCHERBRIDGE_H_
#define EVENTDISPATCHERBRIDGE_H_

#include "PluginInterface.h"
#include "PluginInterfaceSimple.h"

namespace server {
namespace zone {
namespace managers {
namespace plugin {

/**
 * Bridge to convert between full engine types and lightweight plugin types
 */
namespace EventDispatcherBridge {

	inline ChatEventDataLight convertChatEvent(const ChatEventData& data) {
		ChatEventDataLight light;
		light.sender = data.sender.get();
		light.senderName = data.senderName.toCharArray();
		light.senderOID = data.senderOID;
		light.senderAccountID = data.senderAccountID;
		light.message = data.message.toCharArray();
		light.channelType = data.channelType.toCharArray();
		light.recipientName = data.recipientName.toCharArray();
		light.recipientOID = data.recipientOID;
		light.planet = data.planet.toCharArray();
		light.posX = data.posX;
		light.posY = data.posY;
		light.posZ = data.posZ;
		light.timestamp = data.timestamp.getMiliTime();
		return light;
	}
	
	inline PlayerEventDataLight convertPlayerEvent(const PlayerEventData& data) {
		PlayerEventDataLight light;
		light.player = data.player.get();
		light.playerName = data.playerName.toCharArray();
		light.playerOID = data.playerOID;
		light.accountID = data.accountID;
		light.eventType = data.eventType.toCharArray();
		light.details = ""; // Would need to serialize HashTable
		light.planet = "";  // Would need to get from player
		light.posX = 0;
		light.posY = 0;
		light.posZ = 0;
		light.timestamp = data.timestamp.getMiliTime();
		return light;
	}
	
	inline CommandEventDataLight convertCommandEvent(const CommandEventData& data) {
		CommandEventDataLight light;
		light.executor = data.executor.get();
		light.executorName = data.executorName.toCharArray();
		light.executorOID = data.executorOID;
		light.executorAccountID = data.executorAccountID;
		light.command = data.command.toCharArray();
		light.arguments = data.arguments.toCharArray();
		light.target = data.target.get();
		light.targetName = data.targetName.toCharArray();
		light.targetOID = data.targetOID;
		light.result = data.result.toCharArray();
		light.success = data.success;
		light.timestamp = data.timestamp.getMiliTime();
		return light;
	}

/**
 * Wrapper that implements the full interface but delegates to a simple plugin
 */
class PluginWrapper : public IPlugin {
private:
	::plugin::simple::IPlugin* simplePlugin;
	::plugin::simple::IEventListener* eventListener;
	::plugin::simple::ICommandHandler* commandHandler;
	
public:
	PluginWrapper(::plugin::simple::IPlugin* plugin) : simplePlugin(plugin) {
		eventListener = dynamic_cast<::plugin::simple::IEventListener*>(plugin);
		commandHandler = dynamic_cast<::plugin::simple::ICommandHandler*>(plugin);
	}
	
	virtual ~PluginWrapper() {
		// Don't delete simplePlugin - it's managed by the plugin loader
	}
	
	// IPlugin interface
	virtual String getPluginName() const override {
		return String(simplePlugin->getPluginName().c_str());
	}
	
	virtual String getPluginVersion() const override {
		return String(simplePlugin->getPluginVersion().c_str());
	}
	
	virtual void onPluginLoaded() override {
		simplePlugin->onPluginLoaded();
	}
	
	virtual void onPluginUnloaded() override {
		simplePlugin->onPluginUnloaded();
	}
	
	// IEventListener interface
	virtual void onChatEvent(const ChatEventData& data) override {
		if (eventListener) {
			auto lightData = EventDispatcherBridge::convertChatEvent(data);
			eventListener->onChatEvent(lightData);
		}
	}
	
	virtual void onPlayerEvent(const PlayerEventData& data) override {
		if (eventListener) {
			auto lightData = EventDispatcherBridge::convertPlayerEvent(data);
			eventListener->onPlayerEvent(lightData);
		}
	}
	
	virtual void onCommandEvent(const CommandEventData& data) override {
		if (eventListener) {
			auto lightData = EventDispatcherBridge::convertCommandEvent(data);
			eventListener->onCommandEvent(lightData);
		}
	}
	
	virtual void onSystemEvent(const SystemEventData& data) override {
		// Not implemented yet
	}
	
	// ICommandHandler interface
	virtual bool handleCommand(const String& cmd, const CommandParameters& params) override {
		if (commandHandler) {
			CommandParametersLight light;
			light.command = cmd.toCharArray();
			light.executor = params.executor.get();
			light.target = params.target.get();
			light.params = nullptr;
			return commandHandler->handleCommand(cmd.toCharArray(), light);
		}
		return false;
	}
	
	virtual bool canHandleCommand(const String& cmd) const override {
		if (commandHandler) {
			return commandHandler->canHandleCommand(cmd.toCharArray());
		}
		return false;
	}
	
	virtual Vector<String> getSupportedCommands() const override {
		Vector<String> result;
		if (commandHandler) {
			auto commands = commandHandler->getSupportedCommands();
			for (const auto& cmd : commands) {
				result.add(String(cmd.c_str()));
			}
		}
		return result;
	}
	
	virtual int getRequiredAdminLevel(const String& cmd) const override {
		if (commandHandler) {
			return commandHandler->getRequiredAdminLevel(cmd.toCharArray());
		}
		return 0;
	}
	
	// Get the wrapped plugin
	::plugin::simple::IPlugin* getWrappedPlugin() {
		return simplePlugin;
	}
};

} // namespace EventDispatcherBridge

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

#endif // EVENTDISPATCHERBRIDGE_H_