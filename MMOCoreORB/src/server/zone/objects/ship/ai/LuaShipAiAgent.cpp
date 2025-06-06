/*
 * LuaShipAiAgent.cpp
 *
 *  Created on: 24/10/30
 *  Author: Hakry
 */

#include "LuaShipAiAgent.h"

#include <engine/core/ManagedReference.h>
#include <system/lang/ref/Reference.h>
#include <system/lang/String.h>

#include "templates/params/ship/ShipFlag.h"
#include "server/chat/ChatManager.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/ServerCore.h"

#include "server/zone/managers/reaction/ReactionManager.h"
#include "server/zone/objects/ship/ai/ShipAiAgent.h"
#include "server/zone/objects/area/ActiveArea.h"
#include "server/zone/objects/tangible/threat/ThreatMap.h"

const char LuaShipAiAgent::className[] = "LuaShipAiAgent";

Luna<LuaShipAiAgent>::RegType LuaShipAiAgent::Register[] = {
	{ "_setObject", &LuaShipAiAgent::_setObject },
	{ "_getObject", &LuaSceneObject::_getObject },
	{ "setShipAiTemplate", &LuaShipAiAgent::setShipAiTemplate },
	{ "setGuardPatrol", &LuaShipAiAgent::setGuardPatrol },
	{ "setRandomPatrol", &LuaShipAiAgent::setRandomPatrol },
	{ "setFixedPatrol", &LuaShipAiAgent::setFixedPatrol },
	{ "setSquadronPatrol", &LuaShipAiAgent::setSquadronPatrol },
	{ "setSquadronFollow", &LuaShipAiAgent::setSquadronFollow },
	{ "setEscort", &LuaShipAiAgent::setEscort },
	{ "setDespawnOnNoPlayerInRange", &LuaShipAiAgent::setDespawnOnNoPlayerInRange },
	{ "setMinimumGuardPatrol", &LuaShipAiAgent::setMinimumGuardPatrol },
	{ "setMaximumGuardPatrol", &LuaShipAiAgent::setMaximumGuardPatrol },
	{ "addFixedPatrolPoint", &LuaShipAiAgent::addFixedPatrolPoint },
	{ "setDefender", &LuaShipAiAgent::setDefender },
	{ "getShipAgentTemplateName", &LuaShipAiAgent::getShipAgentTemplateName },
	{ "tauntPlayer", &LuaShipAiAgent::tauntPlayer },
	{ "addAggro", &LuaShipAiAgent::addAggro },
	{ "addSpaceFactionAlly", &LuaShipAiAgent::addSpaceFactionAlly },
	{ "removeSpaceFactionAlly", &LuaShipAiAgent::removeSpaceFactionAlly },
	{ "addSpaceFactionEnemy", &LuaShipAiAgent::addSpaceFactionEnemy },
	{ "removeSpaceFactionEnemy", &LuaShipAiAgent::removeSpaceFactionEnemy },
	{ "setEscortSpeed", &LuaShipAiAgent::setEscortSpeed },

	{ 0, 0 }
};

LuaShipAiAgent::LuaShipAiAgent(lua_State *L) : LuaShipObject(L) {
#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<ShipAiAgent*>(_getRealSceneObject());

	E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = static_cast<AiAgent*>(lua_touserdata(L, 1));
#endif
}

LuaShipAiAgent::~LuaShipAiAgent(){
}

int LuaShipAiAgent::_setObject(lua_State* L) {
	LuaShipObject::_setObject(L);

#ifdef DYNAMIC_CAST_LUAOBJECTS
	auto obj = dynamic_cast<ShipAiAgent*>(_getRealSceneObject());

	if (realObject != obj)
		realObject = obj;

	E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	auto obj = static_cast<AiAgent*>(lua_touserdata(L, -1));

	if (realObject != obj)
		realObject = obj;
#endif

	return 0;
}

int LuaShipAiAgent::setShipAiTemplate(lua_State* L) {
	Locker locker(realObject);
	realObject->setShipAiTemplate();

	return 0;
}

int LuaShipAiAgent::setGuardPatrol(lua_State* L) {
	Locker locker(realObject);

	realObject->addShipFlag(ShipFlag::GUARD_PATROL);
	realObject->setShipAiTemplate();

	return 0;
}

int LuaShipAiAgent::setRandomPatrol(lua_State* L) {
	Locker locker(realObject);

	realObject->addShipFlag(ShipFlag::RANDOM_PATROL);
	realObject->setShipAiTemplate();

	return 0;
}

int LuaShipAiAgent::setFixedPatrol(lua_State* L) {
	Locker locker(realObject);

	realObject->addShipFlag(ShipFlag::FIXED_PATROL);
	realObject->setShipAiTemplate();

	return 0;
}

int LuaShipAiAgent::setSquadronPatrol(lua_State* L) {
	Locker locker(realObject);

	realObject->addShipFlag(ShipFlag::SQUADRON_PATROL);
	realObject->setShipAiTemplate();

	return 0;
}

int LuaShipAiAgent::setSquadronFollow(lua_State* L) {
	Locker locker(realObject);

	realObject->addShipFlag(ShipFlag::SQUADRON_FOLLOW);
	realObject->setShipAiTemplate();

	return 0;
}

int LuaShipAiAgent::setEscort(lua_State* L) {
	Locker locker(realObject);

	realObject->addShipFlag(ShipFlag::ESCORT);
	realObject->setShipAiTemplate();

	return 0;
}


int LuaShipAiAgent::setDespawnOnNoPlayerInRange(lua_State* L) {
	bool val = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->setDespawnOnNoPlayerInRange(val);

	return 0;
}

int LuaShipAiAgent::setMinimumGuardPatrol(lua_State* L) {
	float minDist = lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setMinimumGuardPatrol(minDist);

	return 0;
}

int LuaShipAiAgent::setMaximumGuardPatrol(lua_State* L) {
	float maxDist = lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setMaximumGuardPatrol(maxDist);

	return 0;
}

int LuaShipAiAgent::addFixedPatrolPoint(lua_State* L) {
	String name = lua_tostring(L, -1);

	if (name.isEmpty()) {
		return 0;
	}

	Locker locker(realObject);

	realObject->addFixedPatrolPoint(name.hashCode());

	return 0;
}

int LuaShipAiAgent::setDefender(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::setDefender.";
		return 0;
	}

	SceneObject* shipScene = (SceneObject*) lua_touserdata(L, -1);

	if (shipScene == nullptr || !shipScene->isShipObject()) {
		return 0;
	}

	auto shipObject = shipScene->asShipObject();

	if (shipObject == nullptr) {
		return 0;
	}

	Locker locker(realObject);
	Locker clock(shipObject, realObject);

	realObject->setDefender(shipObject);

	return 0;
}

int LuaShipAiAgent::getShipAgentTemplateName(lua_State* L) {
	String templateName = realObject->getShipAgentTemplateName();

	lua_pushstring(L, templateName.toCharArray());

	return 1;
}

int LuaShipAiAgent::tauntPlayer(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 2) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::tauntPlayer.";
		return 0;
	}

	String message = lua_tostring(L, -1);
	CreatureObject* player = (CreatureObject*) lua_touserdata(L, -2);

	if (player == nullptr || !player->isPlayerCreature()) {
		return 0;
	}

	Locker lock(realObject);
	Locker clock(player, realObject);

	realObject->tauntPlayer(player, message);

	return 0;
}

int LuaShipAiAgent::addAggro(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 2) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::addAggro.";
		return 0;
	}

	int aggroValue = lua_tonumber(L, -1);
	TangibleObject* shipTanO = (TangibleObject*) lua_touserdata(L, -2);

	if (shipTanO == nullptr || !shipTanO->isShipObject()) {
		return 0;
	}

	Locker lock(realObject);

	auto ship = shipTanO->asShipObject();
	auto threatMap = realObject->getThreatMap();

	if (ship == nullptr || threatMap == nullptr) {
		return 0;
	}

	Locker clock(shipTanO, realObject);

	threatMap->addAggro(ship, aggroValue);

	return 0;
}

int LuaShipAiAgent::addSpaceFactionAlly(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::addSpaceFactionAlly.";
		return 0;
	}

	uint32 factionHash = lua_tointeger(L, -1);

	Locker lock(realObject);

	realObject->addSpaceFactionAlly(factionHash);

	return 0;
}

int LuaShipAiAgent::removeSpaceFactionAlly(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::removeSpaceFactionAlly.";
		return 0;
	}

	uint32 factionHash = lua_tointeger(L, -1);

	Locker lock(realObject);

	realObject->removeSpaceFactionAlly(factionHash);

	return 0;
}

int LuaShipAiAgent::addSpaceFactionEnemy(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::addSpaceFactionEnemy.";
		return 0;
	}

	uint32 factionHash = lua_tointeger(L, -1);

	Locker lock(realObject);

	realObject->addSpaceFactionEnemy(factionHash);

	return 0;
}

int LuaShipAiAgent::removeSpaceFactionEnemy(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::removeSpaceFactionEnemy.";
		return 0;
	}

	uint32 factionHash = lua_tointeger(L, -1);

	Locker lock(realObject);

	realObject->removeSpaceFactionEnemy(factionHash);

	return 0;
}

int LuaShipAiAgent::setEscortSpeed(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		realObject->error() << "Improper number of arguments in LuaShipAiAgent::setEscortSpeed.";
		return 0;
	}

	float escortSpeed = lua_tonumber(L, -1);

	Locker lock(realObject);

	realObject->setEscortSpeed(escortSpeed);

	return 0;
}
