#ifndef PLUGINLOADER_H_
#define PLUGINLOADER_H_

#include "PluginInterface.h"
#include "EventDispatcher.h"
#include "engine/util/Singleton.h"
#include "system/io/File.h"
#include <dlfcn.h>
#include <dirent.h>

namespace server {
namespace zone {
namespace managers {
namespace plugin {

/**
 * Plugin loader manages loading and unloading of dynamic plugins
 */
class PluginLoader : public Singleton<PluginLoader>, public Logger, public Object {
private:
	class LoadedPlugin : public Object {
	public:
		String filename;
		void* handle;
		Reference<IPlugin*> plugin;
		
		LoadedPlugin() : handle(nullptr) {}
		
		bool toBinaryStream(ObjectOutputStream* stream) {
			return false; // Don't serialize plugins
		}
		
		bool parseFromBinaryStream(ObjectInputStream* stream) {
			return false; // Don't deserialize plugins
		}
	};
	
	Vector<LoadedPlugin> loadedPlugins;
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
			if (loadedPlugins.get(i).filename == filename) {
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
			plugin = createPlugin();
			if (plugin == nullptr) {
				error("createPlugin returned null: " + filename);
				dlclose(handle);
				return false;
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
		EventDispatcher::instance()->registerListener(plugin);
		EventDispatcher::instance()->registerCommandHandler(plugin);
		
		// Store the loaded plugin
		LoadedPlugin loadedPlugin;
		loadedPlugin.filename = filename;
		loadedPlugin.handle = handle;
		loadedPlugin.plugin = plugin;
		loadedPlugins.add(loadedPlugin);
		
		// Notify plugin it's loaded
		try {
			plugin->onPluginLoaded();
		} catch (const Exception& e) {
			error("Exception in onPluginLoaded: " + e.getMessage());
		}
		
		info("Successfully loaded plugin: " + plugin->getPluginName() + " v" + plugin->getPluginVersion(), true);
		return true;
	}
	
	/**
	 * Unload a specific plugin
	 */
	bool unloadPlugin(const String& filename) {
		Locker locker(&pluginMutex);
		
		for (int i = 0; i < loadedPlugins.size(); ++i) {
			LoadedPlugin& loaded = loadedPlugins.get(i);
			if (loaded.filename == filename) {
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
			const LoadedPlugin& loaded = loadedPlugins.get(i);
			if (loaded.plugin != nullptr) {
				result.add(loaded.plugin->getPluginName() + " (" + loaded.filename + ")");
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
			
		LoadedPlugin& loaded = loadedPlugins.get(index);
		
		info("Unloading plugin: " + loaded.filename, true);
		
		// Notify plugin it's being unloaded
		if (loaded.plugin != nullptr) {
			try {
				loaded.plugin->onPluginUnloaded();
			} catch (const Exception& e) {
				error("Exception in onPluginUnloaded: " + e.getMessage());
			}
			
			// Unregister from event dispatcher
			EventDispatcher::instance()->unregisterListener(loaded.plugin);
			EventDispatcher::instance()->unregisterCommandHandler(loaded.plugin);
		}
		
		// Delete the plugin instance
		loaded.plugin = nullptr;
		
		// Unload the shared library
		if (loaded.handle != nullptr) {
			if (dlclose(loaded.handle) != 0) {
				error("Failed to unload plugin library: " + String(dlerror()));
			}
		}
		
		// Remove from list
		loadedPlugins.remove(index);
		
		return true;
	}
};

} // namespace plugin
} // namespace managers
} // namespace zone
} // namespace server

#endif // PLUGINLOADER_H_