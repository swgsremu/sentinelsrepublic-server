/*
 * ShipAgentTemplateManager.h
 *
 *  Created on: 2024/11/01
 *  Author: Hakry
 */

#ifndef ShipAgentTemplateManager_H_
#define ShipAgentTemplateManager_H_

#include "server/zone/objects/ship/ai/ShipAgentTemplate.h"

namespace server {
namespace zone {
namespace managers {
namespace ship {

class ShipAgentTemplateManager : public Singleton<ShipAgentTemplateManager>, public Object, public Logger {
protected:
	Reference<Lua*> lua;
	HashTable<uint32, Reference<ShipAgentTemplate*>> hashTable;

	static AtomicInteger loadedMobileTemplates;
	float globalAttackSpeedOverride;

public:
	static int DEBUG_MODE;
	enum LUA_ERROR_CODE { NO_ERROR = 0, GENERAL_ERROR, DUPLICATE_MOBILE, INCORRECT_ARGUMENTS, DUPLICATE_CONVO };
	static int ERROR_CODE;

	ShipAgentTemplateManager();
	virtual ~ShipAgentTemplateManager();

	int loadTemplates();
	void loadLuaConfig();

	static int includeFile(lua_State* L);
	static int addTemplate(lua_State* L);
	static int checkArgumentCount(lua_State* L, int args);

	int size() {
		return hashTable.size();
	}

	HashTableIterator<uint32, Reference<ShipAgentTemplate*>> iterator() {
		return hashTable.iterator();
	}

	ShipAgentTemplate* getTemplate(uint32 crc) {
		return hashTable.get(crc);
	}

	ShipAgentTemplate* getTemplate(const String& ascii) {
		return hashTable.get(ascii.hashCode());
	}
};

} // namespace ship
} // namespace managers
} // namespace zone
} // namespace server

using namespace server::zone::managers::ship;

#endif /* ShipAgentTemplateManager_H_ */
