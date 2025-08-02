#ifndef EVENTDISPATCHER_H_
#define EVENTDISPATCHER_H_

#include "PluginInterface.h"
#include "engine/util/Singleton.h"
#include "engine/core/ManagedReference.h"

namespace server {
namespace zone {
namespace managers {
namespace plugin {

/**
 * Central event dispatcher for the plugin system
 * Manages event listeners and command handlers
 */
class EventDispatcher : public Singleton<EventDispatcher>, public Logger, public Object {
private:
	Vector<Reference<IEventListener*>> eventListeners;
	Vector<Reference<ICommandHandler*>> commandHandlers;
	mutable Mutex listenerMutex;
	mutable Mutex handlerMutex;
	
public:
	EventDispatcher() {
		setLoggingName("EventDispatcher");
		setGlobalLogging(true);
		setLogging(true);
	}
	
	virtual ~EventDispatcher() {
		clearAllListeners();
	}
	
	/**
	 * Register an event listener
	 */
	void registerListener(IEventListener* listener) {
		if (listener == nullptr)
			return;
			
		Locker locker(&listenerMutex);
		
		// Check if already registered
		for (int i = 0; i < eventListeners.size(); ++i) {
			if (eventListeners.get(i) == listener)
				return;
		}
		
		eventListeners.add(listener);
		info("Registered event listener: " + listener->getPluginName(), true);
	}
	
	/**
	 * Unregister an event listener
	 */
	void unregisterListener(IEventListener* listener) {
		if (listener == nullptr)
			return;
			
		Locker locker(&listenerMutex);
		
		for (int i = 0; i < eventListeners.size(); ++i) {
			if (eventListeners.get(i) == listener) {
				eventListeners.remove(i);
				info("Unregistered event listener: " + listener->getPluginName(), true);
				break;
			}
		}
	}
	
	/**
	 * Register a command handler
	 */
	void registerCommandHandler(ICommandHandler* handler) {
		if (handler == nullptr)
			return;
			
		Locker locker(&handlerMutex);
		
		// Check if already registered
		for (int i = 0; i < commandHandlers.size(); ++i) {
			if (commandHandlers.get(i) == handler)
				return;
		}
		
		commandHandlers.add(handler);
		
		// Log supported commands
		Vector<String> commands = handler->getSupportedCommands();
		StringBuffer msg;
		msg << "Registered command handler supporting: ";
		for (int i = 0; i < commands.size(); ++i) {
			if (i > 0) msg << ", ";
			msg << commands.get(i);
		}
		info(msg.toString(), true);
	}
	
	/**
	 * Unregister a command handler
	 */
	void unregisterCommandHandler(ICommandHandler* handler) {
		if (handler == nullptr)
			return;
			
		Locker locker(&handlerMutex);
		
		for (int i = 0; i < commandHandlers.size(); ++i) {
			if (commandHandlers.get(i) == handler) {
				commandHandlers.remove(i);
				info("Unregistered command handler", true);
				break;
			}
		}
	}
	
	/**
	 * Dispatch a chat event to all listeners
	 */
	void dispatchChatEvent(const ChatEventData& data) {
		Locker locker(&listenerMutex);
		
		for (int i = 0; i < eventListeners.size(); ++i) {
			Reference<IEventListener*> listener = eventListeners.get(i);
			if (listener != nullptr) {
				try {
					listener->onChatEvent(data);
				} catch (const Exception& e) {
					error("Exception in chat event listener: " + e.getMessage());
				} catch (...) {
					error("Unknown exception in chat event listener");
				}
			}
		}
	}
	
	/**
	 * Dispatch a player event to all listeners
	 */
	void dispatchPlayerEvent(const PlayerEventData& data) {
		Locker locker(&listenerMutex);
		
		for (int i = 0; i < eventListeners.size(); ++i) {
			Reference<IEventListener*> listener = eventListeners.get(i);
			if (listener != nullptr) {
				try {
					listener->onPlayerEvent(data);
				} catch (const Exception& e) {
					error("Exception in player event listener: " + e.getMessage());
				} catch (...) {
					error("Unknown exception in player event listener");
				}
			}
		}
	}
	
	/**
	 * Dispatch a command event to all listeners
	 */
	void dispatchCommandEvent(const CommandEventData& data) {
		Locker locker(&listenerMutex);
		
		for (int i = 0; i < eventListeners.size(); ++i) {
			Reference<IEventListener*> listener = eventListeners.get(i);
			if (listener != nullptr) {
				try {
					listener->onCommandEvent(data);
				} catch (const Exception& e) {
					error("Exception in command event listener: " + e.getMessage());
				} catch (...) {
					error("Unknown exception in command event listener");
				}
			}
		}
	}
	
	/**
	 * Dispatch a system event to all listeners
	 */
	void dispatchSystemEvent(const SystemEventData& data) {
		Locker locker(&listenerMutex);
		
		for (int i = 0; i < eventListeners.size(); ++i) {
			Reference<IEventListener*> listener = eventListeners.get(i);
			if (listener != nullptr) {
				try {
					listener->onSystemEvent(data);
				} catch (const Exception& e) {
					error("Exception in system event listener: " + e.getMessage());
				} catch (...) {
					error("Unknown exception in system event listener");
				}
			}
		}
	}
	
	/**
	 * Handle a command through registered handlers
	 * @return true if command was handled
	 */
	bool handleCommand(const String& cmd, const CommandParameters& params) {
		Locker locker(&handlerMutex);
		
		for (int i = 0; i < commandHandlers.size(); ++i) {
			Reference<ICommandHandler*> handler = commandHandlers.get(i);
			if (handler != nullptr && handler->canHandleCommand(cmd)) {
				try {
					return handler->handleCommand(cmd, params);
				} catch (const Exception& e) {
					error("Exception in command handler for " + cmd + ": " + e.getMessage());
				} catch (...) {
					error("Unknown exception in command handler for " + cmd);
				}
			}
		}
		
		return false;
	}
	
	/**
	 * Check if any handler can process the command
	 */
	bool canHandleCommand(const String& cmd) const {
		Locker locker(&handlerMutex);
		
		for (int i = 0; i < commandHandlers.size(); ++i) {
			Reference<ICommandHandler*> handler = commandHandlers.get(i);
			if (handler != nullptr && handler->canHandleCommand(cmd)) {
				return true;
			}
		}
		
		return false;
	}
	
	/**
	 * Get required admin level for a command
	 */
	int getRequiredAdminLevel(const String& cmd) const {
		Locker locker(&handlerMutex);
		
		for (int i = 0; i < commandHandlers.size(); ++i) {
			Reference<ICommandHandler*> handler = commandHandlers.get(i);
			if (handler != nullptr && handler->canHandleCommand(cmd)) {
				return handler->getRequiredAdminLevel(cmd);
			}
		}
		
		return 0;
	}
	
	/**
	 * Clear all listeners and handlers
	 */
	void clearAllListeners() {
		{
			Locker locker(&listenerMutex);
			eventListeners.removeAll();
		}
		{
			Locker locker(&handlerMutex);
			commandHandlers.removeAll();
		}
		info("Cleared all event listeners and command handlers", true);
	}
	
	/**
	 * Get number of registered listeners
	 */
	int getListenerCount() const {
		Locker locker(&listenerMutex);
		return eventListeners.size();
	}
	
	/**
	 * Get number of registered handlers
	 */
	int getHandlerCount() const {
		Locker locker(&handlerMutex);
		return commandHandlers.size();
	}
};

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

#endif // EVENTDISPATCHER_H_