/*
 * ActionManager.h
 *
 * Registry and factory for actions using static self-registration
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "system/lang.h"
#include "ActionBase.h"

/**
 * Action registry and factory
 *
 * Uses Meyers singleton pattern for thread-safe static initialization
 * Actions register themselves via static initializers at link time
 *
 * Example registration in action .cpp file:
 *   static bool _registered_myAction =
 *       (ActionManager::registerAction("myAction", MyAction::factory), true);
 */
class ActionManager {
public:
	/**
	 * Register an action factory function
	 *
	 * @param name Action name (e.g., "loginAccount")
	 * @param factory Function pointer that creates action instances
	 *
	 * Called by static initializers before main()
	 */
	static void registerAction(const char* name, ActionBase* (*factory)());

	/**
	 * Create an action instance by name
	 *
	 * @param name Action name to create
	 * @return New action instance, or nullptr if not registered
	 *
	 * Caller is responsible for deleting the returned action
	 */
	static ActionBase* createAction(const char* name);

	/**
	 * List all registered action names
	 *
	 * @return Vector of action names
	 *
	 * Useful for introspection and help text generation
	 */
	static Vector<String> listActions();

	/**
	 * Check if an action is registered
	 *
	 * @param name Action name to check
	 * @return true if action is registered
	 */
	static bool hasAction(const char* name);

private:
	/**
	 * Meyers singleton for thread-safe registry initialization
	 * Registry is a map of action name -> factory function
	 */
	static VectorMap<String, ActionBase* (*)()>& getRegistry() {
		static VectorMap<String, ActionBase* (*)()> registry;
		return registry;
	}
};

#endif /* ACTIONMANAGER_H_ */
