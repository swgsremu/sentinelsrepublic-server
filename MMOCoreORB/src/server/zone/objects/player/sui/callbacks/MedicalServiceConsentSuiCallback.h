/*
 * MedicalServiceConsentSuiCallback.h
 *
 *  Created on: Oct 26, 2025
 *      Author: Codex
 */

#ifndef MEDICALSERVICECONSENTSUICALLBACK_H_
#define MEDICALSERVICECONSENTSUICALLBACK_H_

#include "server/zone/objects/player/sui/SuiCallback.h"

namespace server {
namespace zone {
class ZoneServer;
namespace objects {
namespace creature {
	class CreatureObject;
}
namespace player {
namespace sessions {
	class MedicalServiceSession;
}
}
}
}
}

class MedicalServiceConsentSuiCallback : public SuiCallback {
public:
	enum ServiceType {
		ENHANCEMENTS = 1,
		WOUND = 2,
		ENHANCEMENTS_WITH_RESISTS = 3,
		ENTERTAINER = 4,
		PET = 5
	};

private:
	ManagedReference<server::zone::objects::creature::CreatureObject*> practitioner;
	int serviceType;

public:
	MedicalServiceConsentSuiCallback(server::zone::ZoneServer* server,
			server::zone::objects::creature::CreatureObject* practitioner,
			int serviceType);

	virtual void run(server::zone::objects::creature::CreatureObject* player, SuiBox* suiBox,
			uint32 eventIndex, Vector<UnicodeString>* args);
};

#endif /* MEDICALSERVICECONSENTSUICALLBACK_H_ */
