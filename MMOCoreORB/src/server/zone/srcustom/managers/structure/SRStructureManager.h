#ifndef SRSTRUCTUREMANAGER_H_
#define SRSTRUCTUREMANAGER_H_

#include <server/zone/managers/structure/StructureManager.h>
#include "server/zone/objects/creature/CreatureObject.h"

class StructureManager;

class SRStructureManager : public ManagedObject, public Logger
{
private:
	StructureManager* structureManager = nullptr;

public:
	SRStructureManager() = default;

	static const String DATAPAD_FULL_MESSAGE;
    static const String BUILDING_NULL_OR_UNLOAD_FAILED_MESSAGE;
    static const String SUCCESS_MESSAGE;

    // Log message components
    static const String LOG_PREFIX;
    static const String LOG_OWNED_BY;
    static const String LOG_SUFFIX;
	static const uint32 CONTROL_DEVICE_HASH;

	void setStructureManager(StructureManager* manager);

	int packupStructure(CreatureObject* creature);
	// bool readObjectMember(ObjectInputStream* stream, const String& name);
	// void systemPackupStructure(StructureObject* structure, CreatureObject* creature);
	// int unpackStructureFromControlDevice(CreatureObject* creature, StructureObject* structure, float x, float y, int angle);
	// bool unpackStructure(CreatureObject* creature, StructureObject* structure, float x, float y, int angle, int persistenceLevel = 1);
};

#endif /* SRSTRUCTUREMANAGER_H_ */
