/*
 * ActionManager.cpp
 *
 * Implementation of action registry and factory
 */

#include "ActionManager.h"

void ActionManager::registerAction(const char* name, ActionBase* (*factory)()) {
	getRegistry().put(String(name), factory);
}

ActionBase* ActionManager::createAction(const char* name) {
	auto& registry = getRegistry();
	String actionName(name);

	if (!registry.contains(actionName)) {
		return nullptr;
	}

	// Call factory function to create new instance
	return registry.get(actionName)();
}

Vector<String> ActionManager::listActions() {
	Vector<String> names;
	auto& registry = getRegistry();

	for (int i = 0; i < registry.size(); i++) {
		names.add(registry.elementAt(i).getKey());
	}

	return names;
}

bool ActionManager::hasAction(const char* name) {
	auto& registry = getRegistry();
	return registry.contains(String(name));
}
