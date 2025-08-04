#ifndef PLUGINLOADER_H_
#define PLUGINLOADER_H_

#include "PluginInterface.h"
#include "EventDispatcher.h"
#include "EventDispatcherBridge.h"
#include "engine/util/Singleton.h"
#include "system/io/File.h"
#include <dlfcn.h>
#include <dirent.h>

namespace server {
namespace zone {
namespace managers {
namespace plugin {

using PluginWrapper = EventDispatcherBridge::PluginWrapper;

/**
 * Plugin loader manages loading and unloading of dynamic plugins
 */
class PluginLoader : public Singleton<PluginLoader>, public Logger, public Object {
private:
	struct LoadedPlugin {
		String filename;
		void* handle;
		IPlugin* plugin;
		
		LoadedPlugin() : handle(nullptr), plugin(nullptr) {}
	};
	
	Vector<LoadedPlugin*> loadedPlugins;
	mutable Mutex pluginMutex;
	
public:
	PluginLoader() {
		setLoggingName("PluginLoader");
		setGlobalLogging(true);
		setLogging(true);
	}
	
	virtual ~PluginLoader() {
		unloadAllPlugins();
	}
	
	/**
	 * Load all plugins from a directory
	 */
	void loadPlugins(const String& directory) {
		info("Loading plugins from: " + directory, true);
		
		DIR* dir = opendir(directory.toCharArray());
		if (dir == nullptr) {
			warning("Plugin directory does not exist or cannot be opened: " + directory);
			return;
		}
		
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			String filename = entry->d_name;
			
			// Skip . and ..
			if (filename == "." || filename == "..")
				continue;
			
			// Only load .so files
			if (filename.endsWith(".so")) {
				String fullPath = directory + "/" + filename;
				loadPlugin(fullPath);
			}
		}
		
		closedir(dir);
		
		info("Loaded " + String::valueOf(loadedPlugins.size()) + " plugins", true);
	}
	
	/**
	 * Load a single plugin
	 */
	bool loadPlugin(const String& filename) {
		Locker locker(&pluginMutex);
		
		// Check if already loaded
		for (int i = 0; i < loadedPlugins.size(); ++i) {
			if (loadedPlugins.get(i)->filename == filename) {
				warning("Plugin already loaded: " + filename);
				return false;
			}
		}
		
		info("Loading plugin: " + filename, true);
		
		// Load the shared library
		void* handle = dlopen(filename.toCharArray(), RTLD_LAZY | RTLD_LOCAL);
		if (handle == nullptr) {
			error("Failed to load plugin: " + String(dlerror()));
			return false;
		}
		
		// Get the create function
		typedef IPlugin* (*CreatePluginFunc)();
		CreatePluginFunc createPlugin = (CreatePluginFunc) dlsym(handle, "createPlugin");
		
		if (createPlugin == nullptr) {
			error("Plugin does not export createPlugin function: " + filename);
			dlclose(handle);
			return false;
		}
		
		// Create the plugin instance
		IPlugin* plugin = nullptr;
		try {
			// Try to get the simple plugin interface first
			typedef ::plugin::simple::IPlugin* (*CreateSimplePluginFunc)();
			CreateSimplePluginFunc createSimplePlugin = (CreateSimplePluginFunc) dlsym(handle, "createPlugin");
			
			if (createSimplePlugin != nullptr) {
				// This is a simple plugin, wrap it
				auto simplePlugin = createSimplePlugin();
				if (simplePlugin == nullptr) {
					error("createPlugin returned null: " + filename);
					dlclose(handle);
					return false;
				}
				// Wrap the simple plugin with our adapter
				plugin = new PluginWrapper(simplePlugin);
			} else {
				// Try legacy interface
				plugin = createPlugin();
				if (plugin == nullptr) {
					error("createPlugin returned null: " + filename);
					dlclose(handle);
					return false;
				}
			}
		} catch (const Exception& e) {
			error("Exception creating plugin: " + e.getMessage());
			dlclose(handle);
			return false;
		} catch (...) {
			error("Unknown exception creating plugin");
			dlclose(handle);
			return false;
		}
		
		// Register with event dispatcher
		info("Registering plugin as event listener and command handler", true);
		EventDispatcher::instance()->registerListener(plugin);
		EventDispatcher::instance()->registerCommandHandler(plugin);
		info("Plugin registered successfully", true);
		
		// Store the loaded plugin
		LoadedPlugin* loadedPlugin = new LoadedPlugin();
		loadedPlugin->filename = filename;
		loadedPlugin->handle = handle;
		loadedPlugin->plugin = plugin;
		loadedPlugins.add(loadedPlugin);
		
		// Notify plugin it's loaded
		try {
			plugin->onPluginLoaded();
		} catch (const Exception& e) {
			error("Exception in onPluginLoaded: " + e.getMessage());
		}
		
		info("Successfully loaded plugin: " + plugin->getPluginName() + " v" + plugin->getPluginVersion(), true);
		
		// Test dispatch a chat event
		info("Testing chat event dispatch to plugin", true);
		server::zone::managers::plugin::ChatEventData testData;
		testData.senderName = "TestSender";
		testData.message = "Test message from PluginLoader";
		testData.channelType = "TEST";
		EventDispatcher::instance()->dispatchChatEvent(testData);
		return true;
	}
	
	/**
	 * Unload a specific plugin
	 */
	bool unloadPlugin(const String& filename) {
		Locker locker(&pluginMutex);
		
		for (int i = 0; i < loadedPlugins.size(); ++i) {
			LoadedPlugin* loaded = loadedPlugins.get(i);
			if (loaded->filename == filename) {
				return unloadPluginAt(i);
			}
		}
		
		warning("Plugin not found: " + filename);
		return false;
	}
	
	/**
	 * Unload all plugins
	 */
	void unloadAllPlugins() {
		Locker locker(&pluginMutex);
		
		info("Unloading all plugins", true);
		
		while (loadedPlugins.size() > 0) {
			unloadPluginAt(loadedPlugins.size() - 1);
		}
	}
	
	/**
	 * Reload a plugin
	 */
	bool reloadPlugin(const String& filename) {
		info("Reloading plugin: " + filename, true);
		
		if (!unloadPlugin(filename)) {
			return false;
		}
		
		return loadPlugin(filename);
	}
	
	/**
	 * Get list of loaded plugins
	 */
	Vector<String> getLoadedPlugins() const {
		Locker locker(&pluginMutex);
		
		Vector<String> result;
		for (int i = 0; i < loadedPlugins.size(); ++i) {
			LoadedPlugin* loaded = loadedPlugins.get(i);
			if (loaded->plugin != nullptr) {
				result.add(loaded->plugin->getPluginName() + " (" + loaded->filename + ")");
			}
		}
		
		return result;
	}
	
private:
	/**
	 * Unload plugin at specific index
	 */
	bool unloadPluginAt(int index) {
		if (index < 0 || index >= loadedPlugins.size())
			return false;
			
		LoadedPlugin* loaded = loadedPlugins.get(index);
		
		info("Unloading plugin: " + loaded->filename, true);
		
		// Notify plugin it's being unloaded
		if (loaded->plugin != nullptr) {
			try {
				loaded->plugin->onPluginUnloaded();
			} catch (const Exception& e) {
				error("Exception in onPluginUnloaded: " + e.getMessage());
			}
			
			// Unregister from event dispatcher
			EventDispatcher::instance()->unregisterListener(loaded->plugin);
			EventDispatcher::instance()->unregisterCommandHandler(loaded->plugin);
		}
		
		// Delete the plugin instance
		// Check if it's a wrapped plugin
		auto wrapper = dynamic_cast<PluginWrapper*>(loaded->plugin);
		if (wrapper != nullptr) {
			// Get the wrapped plugin before deleting wrapper
			auto simplePlugin = wrapper->getWrappedPlugin();
			delete wrapper;
			// Now destroy the simple plugin
			typedef void (*DestroySimplePluginFunc)(::plugin::simple::IPlugin*);
			DestroySimplePluginFunc destroyPlugin = (DestroySimplePluginFunc) dlsym(loaded->handle, "destroyPlugin");
			if (destroyPlugin != nullptr) {
				destroyPlugin(simplePlugin);
			}
		} else {
			// Legacy plugin
			delete loaded->plugin;
		}
		loaded->plugin = nullptr;
		
		// Unload the shared library
		if (loaded->handle != nullptr) {
			if (dlclose(loaded->handle) != 0) {
				error("Failed to unload plugin library: " + String(dlerror()));
			}
		}
		
		// Remove from list and delete the LoadedPlugin object
		delete loaded;
		loadedPlugins.remove(index);
		
		return true;
	}
};

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

#endif // PLUGINLOADER_H_