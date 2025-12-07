#include "server/zone/objects/player/sui/callbacks/MedicalServiceConsentSuiCallback.h"

#include "server/zone/objects/player/sessions/MedicalServiceSession.h"
#include "server/zone/objects/scene/SessionFacadeType.h"
#include "server/zone/objects/player/sui/SuiBox.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/ZoneServer.h"

using namespace server::zone;
using namespace server::zone::objects::creature;
using namespace server::zone::objects::player::sessions;

MedicalServiceConsentSuiCallback::MedicalServiceConsentSuiCallback(ZoneServer* server,
		CreatureObject* practitioner, int serviceType)
	: SuiCallback(server) {
	this->practitioner = practitioner;
	this->serviceType = serviceType;
}

void MedicalServiceConsentSuiCallback::run(CreatureObject* player, SuiBox* suiBox, uint32 eventIndex,
		Vector<UnicodeString>* args) {
	if (player == nullptr || suiBox == nullptr)
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	ManagedReference<MedicalServiceSession*> serviceSession =
			doctor->getActiveSession(SessionFacadeType::MEDICALSERVICE).castTo<MedicalServiceSession*>();

	if (serviceSession == nullptr)
		return;

	bool accepted = (eventIndex == 0);

	switch (serviceType) {
	case ENHANCEMENTS:
		serviceSession->confirmEnhancementRequest(player, accepted);
		break;
	case ENHANCEMENTS_WITH_RESISTS:
		serviceSession->confirmEnhancementResistRequest(player, accepted);
		break;
	case ENTERTAINER:
		serviceSession->confirmEntertainerBuffRequest(player, accepted);
		break;
	case PET:
		serviceSession->confirmPetBuffRequest(player, accepted);
		break;
	default:
		serviceSession->confirmWoundRequest(player, accepted);
		break;
	}
}
