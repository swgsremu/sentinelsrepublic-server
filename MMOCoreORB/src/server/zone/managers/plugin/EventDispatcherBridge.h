#ifndef EVENTDISPATCHERBRIDGE_H_
#define EVENTDISPATCHERBRIDGE_H_

#include "PluginInterface.h"
#include "PluginInterfaceSimple.h"
#include <string>

namespace server {
namespace zone {
namespace managers {
namespace plugin {

/**
 * Bridge to convert between full engine types and lightweight plugin types
 */
namespace EventDispatcherBridge {

	// Store strings to prevent dangling pointers
	struct ChatEventStrings {
		std::string senderName;
		std::string message;
		std::string channelType;
		std::string recipientName;
		std::string planet;
	};
	
	inline ChatEventDataLight convertChatEvent(const ChatEventData& data, ChatEventStrings& strings) {
		// Store strings in the provided structure
		strings.senderName = std::string(data.senderName.toCharArray());
		strings.message = std::string(data.message.toCharArray());
		strings.channelType = std::string(data.channelType.toCharArray());
		strings.recipientName = std::string(data.recipientName.toCharArray());
		strings.planet = std::string(data.planet.toCharArray());
		
		ChatEventDataLight light;
		light.sender = data.sender.get();
		light.senderName = strings.senderName.c_str();
		light.senderOID = data.senderOID;
		light.senderAccountID = data.senderAccountID;
		light.message = strings.message.c_str();
		light.channelType = strings.channelType.c_str();
		light.recipientName = strings.recipientName.c_str();
		light.recipientOID = data.recipientOID;
		light.planet = strings.planet.c_str();
		light.posX = data.posX;
		light.posY = data.posY;
		light.posZ = data.posZ;
		light.timestamp = data.timestamp.getMiliTime();
		return light;
	}
	
	struct PlayerEventStrings {
		std::string playerName;
		std::string eventType;
		std::string details;
		std::string planet;
	};
	
	inline PlayerEventDataLight convertPlayerEvent(const PlayerEventData& data, PlayerEventStrings& strings) {
		// Store strings in the provided structure
		strings.playerName = std::string(data.playerName.toCharArray());
		strings.eventType = std::string(data.eventType.toCharArray());
		strings.details = ""; // Would need to serialize HashTable
		strings.planet = "";  // Would need to get from player
		
		PlayerEventDataLight light;
		light.player = data.player.get();
		light.playerName = strings.playerName.c_str();
		light.playerOID = data.playerOID;
		light.accountID = data.accountID;
		light.eventType = strings.eventType.c_str();
		light.details = strings.details.c_str();
		light.planet = strings.planet.c_str();
		light.posX = 0;
		light.posY = 0;
		light.posZ = 0;
		light.timestamp = data.timestamp.getMiliTime();
		return light;
	}
	
	struct CommandEventStrings {
		std::string executorName;
		std::string command;
		std::string arguments;
		std::string targetName;
		std::string result;
	};
	
	inline CommandEventDataLight convertCommandEvent(const CommandEventData& data, CommandEventStrings& strings) {
		// Store strings in the provided structure
		strings.executorName = std::string(data.executorName.toCharArray());
		strings.command = std::string(data.command.toCharArray());
		strings.arguments = std::string(data.arguments.toCharArray());
		strings.targetName = std::string(data.targetName.toCharArray());
		strings.result = std::string(data.result.toCharArray());
		
		CommandEventDataLight light;
		light.executor = data.executor.get();
		light.executorName = strings.executorName.c_str();
		light.executorOID = data.executorOID;
		light.executorAccountID = data.executorAccountID;
		light.command = strings.command.c_str();
		light.arguments = strings.arguments.c_str();
		light.target = data.target.get();
		light.targetName = strings.targetName.c_str();
		light.targetOID = data.targetOID;
		light.result = strings.result.c_str();
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
		Logger::console.info("PluginWrapper::onChatEvent called", true);
		if (eventListener) {
			Logger::console.info("Calling eventListener->onChatEvent", true);
			ChatEventStrings strings;
			auto lightData = EventDispatcherBridge::convertChatEvent(data, strings);
			eventListener->onChatEvent(lightData);
		} else {
			Logger::console.info("eventListener is null", true);
		}
	}
	
	virtual void onPlayerEvent(const PlayerEventData& data) override {
		if (eventListener) {
			PlayerEventStrings strings;
			auto lightData = EventDispatcherBridge::convertPlayerEvent(data, strings);
			eventListener->onPlayerEvent(lightData);
		}
	}
	
	virtual void onCommandEvent(const CommandEventData& data) override {
		if (eventListener) {
			CommandEventStrings strings;
			auto lightData = EventDispatcherBridge::convertCommandEvent(data, strings);
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