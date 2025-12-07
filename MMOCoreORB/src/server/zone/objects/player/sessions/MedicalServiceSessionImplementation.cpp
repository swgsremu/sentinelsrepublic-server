#include "server/zone/objects/player/sessions/MedicalServiceSession.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/MedicalServiceConsentSuiCallback.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/PlayerBitmasks.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/pharmaceutical/PharmaceuticalObject.h"
#include "server/zone/objects/tangible/pharmaceutical/EnhancePack.h"
#include "server/zone/objects/tangible/pharmaceutical/WoundPack.h"
#include "server/zone/objects/creature/BuffAttribute.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "system/lang/String.h"
#include "system/lang/StringBuffer.h"
#include "system/lang/System.h"
#include "system/lang/UnicodeString.h"
#include "system/util/Vector.h"
#include "engine/engine.h"

namespace {
	const int DEFAULT_RANGE_METERS = 7;
	const int DEFAULT_TICKET_DURATION_MS = 600000; // 10 minutes
	const int AUTO_SERVICE_MAX_WAIT_ATTEMPTS = 60;
	const int AUTO_SERVICE_RANGE_RETRY_DELAY = 1000;
	const int AUTO_SERVICE_COOLDOWN_RETRY_DELAY = 500;
	const int AUTO_SERVICE_NEXT_STEP_DELAY = 1000;
	const int AUTO_SERVICE_MIND_RETRY_DELAY = 2000;
	const String SUPPLY_FAILURE_PATIENT_MESSAGE = "This hospital or clinic does not have the required medical supplies to complete your request. You have not been charged.";
	const String LOW_MIND_MESSAGE_PATIENT = "The practitioner needs a moment to recover before continuing your treatment.";
	const String LOW_MIND_MESSAGE_DOCTOR = "You are too mentally exhausted to continue the queued treatments. Rest briefly to recover.";

	inline bool isPlayerCreature(ManagedReference<CreatureObject*> creature) {
		return creature != nullptr && creature->isPlayerCreature();
	}

	inline String buildAutoTaskKey(int serviceFlag, uint64 patientId) {
		StringBuffer key;
		key << "auto_medical_";

		if ((serviceFlag & MedicalServiceSession::SERVICE_PET) != 0)
			key << "pet_";
		else if ((serviceFlag & MedicalServiceSession::SERVICE_ENHANCEMENT) != 0)
			key << "enh_";
		else
			key << "wnd_";

		key << patientId;

		return key.toString();
	}

	void buildEnhancementAttributeList(Vector<String>& attributes, bool includeResists) {
		attributes.add("health");
		attributes.add("action");
		attributes.add("strength");
		attributes.add("constitution");
		attributes.add("quickness");
		attributes.add("stamina");

		if (includeResists) {
			attributes.add("poison");
			attributes.add("disease");
		}
	}

	void buildEntertainerAttributeList(Vector<String>& attributes) {
		attributes.add("action");
		attributes.add("stamina");
		attributes.add("quickness");
	}

	void buildWoundAttributeList(Vector<String>& attributes) {
		attributes.add("health");
		attributes.add("action");
		attributes.add("strength");
		attributes.add("constitution");
		attributes.add("quickness");
		attributes.add("stamina");
	}

	void sendSupplyFailureMessages(CreatureObject* doctor, CreatureObject* patient, const String& patientMessage, const String& doctorMessage) {
		if (patient != nullptr)
			patient->sendSystemMessage(patientMessage);

		if (doctor != nullptr)
			doctor->sendSystemMessage(doctorMessage);
	}

	bool hasEnhancementSupplies(CreatureObject* doctor, const Vector<String>& attributeNames) {
		if (doctor == nullptr || attributeNames.isEmpty())
			return true;

		SceneObject* inventory = doctor->getSlottedObject("inventory");

		if (inventory == nullptr)
			return false;

		int medicineUse = doctor->getSkillMod("healing_ability");

		const int maxAttribute = BuffAttribute::UNKNOWN;
		bool required[BuffAttribute::UNKNOWN + 1] = { false };
		bool available[BuffAttribute::UNKNOWN + 1] = { false };

		int requiredCount = 0;

		for (int i = 0; i < attributeNames.size(); ++i) {
			uint8 attribute = BuffAttribute::getAttribute(attributeNames.get(i));

			if (attribute > maxAttribute || attribute == BuffAttribute::UNKNOWN)
				continue;

			if (!required[attribute]) {
				required[attribute] = true;
				requiredCount++;
			}
		}

		if (requiredCount == 0)
			return true;

		for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
			SceneObject* object = inventory->getContainerObject(i);

			if (object == nullptr || !object->isPharmaceuticalObject())
				continue;

			PharmaceuticalObject* pharma = cast<PharmaceuticalObject*>(object);

			if (pharma == nullptr || !pharma->isEnhancePack())
				continue;

			EnhancePack* pack = cast<EnhancePack*>(pharma);

			if (pack == nullptr)
				continue;

			if (pack->getUseCount() <= 0 || pack->getMedicineUseRequired() > medicineUse)
				continue;

			uint8 attribute = pack->getAttribute();

			if (attribute <= maxAttribute && attribute != BuffAttribute::UNKNOWN && required[attribute])
				available[attribute] = true;
		}

		for (int attribute = 0; attribute <= maxAttribute; ++attribute) {
			if (required[attribute] && !available[attribute])
				return false;
		}

		return true;
	}

	bool hasWoundSupplies(CreatureObject* doctor, const Vector<String>& attributeNames) {
		if (doctor == nullptr || attributeNames.isEmpty())
			return true;

		SceneObject* inventory = doctor->getSlottedObject("inventory");

		if (inventory == nullptr)
			return false;

		int medicineUse = doctor->getSkillMod("healing_ability");

		const int maxAttribute = CreatureAttribute::WILLPOWER;
		std::vector<bool> required(maxAttribute + 1, false);
		std::vector<bool> available(maxAttribute + 1, false);

		int requiredCount = 0;

		for (int i = 0; i < attributeNames.size(); ++i) {
			uint8 attribute = CreatureAttribute::getAttribute(attributeNames.get(i));

			if (attribute > maxAttribute || attribute == CreatureAttribute::UNKNOWN)
				continue;

			if (!required[attribute]) {
				required[attribute] = true;
				requiredCount++;
			}
		}

		if (requiredCount == 0)
			return true;

		for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
			SceneObject* object = inventory->getContainerObject(i);

			if (object == nullptr || !object->isPharmaceuticalObject())
				continue;

			PharmaceuticalObject* pharma = cast<PharmaceuticalObject*>(object);

			if (pharma == nullptr || !pharma->isWoundPack())
				continue;

			WoundPack* pack = cast<WoundPack*>(pharma);

			if (pack == nullptr)
				continue;

			if (pack->getUseCount() <= 0 || pack->getMedicineUseRequired() > medicineUse)
				continue;

			uint8 attribute = pack->getAttribute();

			if (attribute <= maxAttribute && attribute != CreatureAttribute::UNKNOWN && required[attribute])
				available[attribute] = true;
		}

		for (int attribute = 0; attribute <= maxAttribute; ++attribute) {
			if (required[attribute] && !available[attribute])
				return false;
	}

	return true;
}

	bool collectOrganicPetIds(CreatureObject* patient, Vector<uint64>& petIds) {
		if (patient == nullptr)
			return false;

		PlayerObject* ghost = patient->getPlayerObject();

		if (ghost == nullptr)
			return false;

		for (int i = 0; i < ghost->getActivePetsSize(); ++i) {
			ManagedReference<AiAgent*> pet = ghost->getActivePet(i);

			if (pet == nullptr)
				continue;

			if (pet->isDroidObject())
				continue;

			if (pet->getZone() == nullptr)
				continue;

			petIds.add(pet->getObjectID());
		}

		return !petIds.isEmpty();
	}

	bool isCreatureAvailableForAutomation(CreatureObject* creature) {
		if (creature == nullptr)
			return false;

		if (creature->getZone() == nullptr)
			return false;

		if (!creature->isPlayerCreature())
			return true;

		Reference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return false;

		return !ghost->isLinkDead();
	}

	class AutoMedicalServiceTask : public Task {
	private:
		ManagedReference<MedicalServiceSession*> session;
		ManagedReference<CreatureObject*> doctor;
		ManagedReference<CreatureObject*> patient;
		Vector<String> attributes;
		unsigned int commandCRC;
		String taskKey;
		int index;
		int waitCount;
		int serviceFlag;
		bool lowMindNotified;

	public:
		AutoMedicalServiceTask(MedicalServiceSession* session, CreatureObject* doctor, CreatureObject* patient,
				int serviceFlag, bool includeResists, const String& taskKey, const Vector<String>* attributeOverride = nullptr)
				: Task() {
			this->session = session;
			this->doctor = doctor;
			this->patient = patient;
			this->serviceFlag = serviceFlag;
			this->taskKey = taskKey;
			index = 0;
			waitCount = 0;
			lowMindNotified = false;

			if ((serviceFlag & MedicalServiceSession::SERVICE_ENHANCEMENT) != 0) {
				commandCRC = STRING_HASHCODE("healenhance");
				bool useResists = includeResists || ((serviceFlag & MedicalServiceSession::SERVICE_ENHANCEMENT_RESIST) != 0);

				if (attributeOverride != nullptr)
					attributes = *attributeOverride;
				else
					buildEnhancementAttributeList(attributes, useResists);
			} else {
				commandCRC = STRING_HASHCODE("healwound");
				if (attributeOverride != nullptr)
					attributes = *attributeOverride;
				else
					buildWoundAttributeList(attributes);
			}

			setTaskName("AutoMedicalServiceTask");
		}

		const String& getTaskKey() const {
			return taskKey;
		}

		void run() override {
			ManagedReference<MedicalServiceSession*> sessionRef = session.get();
			ManagedReference<CreatureObject*> doctorRef = doctor.get();
			ManagedReference<CreatureObject*> patientRef = patient.get();

			if (sessionRef == nullptr || doctorRef == nullptr || patientRef == nullptr)
				return;

			// Validate objects still exist in zones (prevent crash if logged out during delay)
			if (doctorRef->getZone() == nullptr || patientRef->getZone() == nullptr)
				return;

			Locker doctorLocker(doctorRef);
			Locker patientLocker(patientRef, doctorRef);

			if (attributes.isEmpty())
				return;

			doctorRef->removePendingTask(taskKey);

			bool doctorAvailable = isCreatureAvailableForAutomation(doctorRef);
			bool patientAvailable = isCreatureAvailableForAutomation(patientRef);

			if (!doctorAvailable || !patientAvailable) {
				if (!patientAvailable) {
					sessionRef->revokeAllAccess(patientRef, false);

					if (doctorAvailable && doctorRef != nullptr) {
						StringBuffer docMsg;
						docMsg << patientRef->getFirstName() << " is no longer available for medical treatment.";
						doctorRef->sendSystemMessage(docMsg.toString());
					}
				}

				return;
			}

			if (!sessionRef->isActive())
				return;

			bool hasAccess = false;

			if ((serviceFlag & MedicalServiceSession::SERVICE_ENHANCEMENT) != 0)
				hasAccess = sessionRef->patientHasEnhancementAccess(patientRef, false);
			else
				hasAccess = sessionRef->patientHasWoundAccess(patientRef, false);

			if (!hasAccess)
				return;

			if (!sessionRef->isWithinServiceRange(patientRef)) {
				if (++waitCount > AUTO_SERVICE_MAX_WAIT_ATTEMPTS) {
					patientRef->sendSystemMessage("You moved out of range before your treatment could complete.");

					StringBuffer docMsg;
					docMsg << patientRef->getFirstName() << " moved out of range before the treatment could complete.";
					doctorRef->sendSystemMessage(docMsg.toString());

					sessionRef->revokeAllAccess(patientRef, true);
					return;
				}

				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_RANGE_RETRY_DELAY);
				return;
			}

			int baseMindCost = ((serviceFlag & MedicalServiceSession::SERVICE_WOUND) != 0) ? 50 : 150;
			int requiredMind = doctorRef->calculateCostAdjustment(CreatureAttribute::FOCUS, baseMindCost);

			if (doctorRef->getHAM(CreatureAttribute::MIND) < requiredMind) {
				if (!lowMindNotified) {
					patientRef->sendSystemMessage(LOW_MIND_MESSAGE_PATIENT);
					doctorRef->sendSystemMessage(LOW_MIND_MESSAGE_DOCTOR);
					lowMindNotified = true;
				}

				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_MIND_RETRY_DELAY);
				return;
			}

			lowMindNotified = false;

			// Validate index bounds
			if (index < 0 || index >= attributes.size()) {
				if ((serviceFlag & MedicalServiceSession::SERVICE_ENHANCEMENT) != 0)
					sessionRef->completeEnhancement(patientRef, true);
				else
					sessionRef->completeWound(patientRef, true);

				return;
			}

			if (!doctorRef->canTreatWounds() || doctorRef->isInCombat()) {
				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_COOLDOWN_RETRY_DELAY);
				return;
			}

			String attribute = attributes.get(index);
			doctorRef->executeObjectControllerAction(commandCRC, patientRef->getObjectID(), UnicodeString(attribute));
			index++;
			waitCount = 0;

			doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
		}
	};

	class AutoPetEnhancementTask : public Task {
	private:
		ManagedReference<MedicalServiceSession*> session;
		ManagedReference<CreatureObject*> doctor;
		ManagedReference<CreatureObject*> patient;
		Vector<uint64> petIds;
		Vector<String> attributes;
		String taskKey;
		int petIndex;
		int attributeIndex;
		int waitCount;
		bool lowMindNotified;

	public:
		AutoPetEnhancementTask(MedicalServiceSession* session, CreatureObject* doctor, CreatureObject* patient,
				const Vector<uint64>& pets, const String& taskKey)
				: Task() {
			this->session = session;
			this->doctor = doctor;
			this->patient = patient;
			this->taskKey = taskKey;
			petIds = pets;
			petIndex = 0;
			attributeIndex = 0;
			waitCount = 0;
			lowMindNotified = false;

			buildEnhancementAttributeList(attributes, false);

			setTaskName("AutoPetEnhancementTask");
		}

		const String& getTaskKey() const {
			return taskKey;
		}

		void run() override {
			ManagedReference<MedicalServiceSession*> sessionRef = session.get();
			ManagedReference<CreatureObject*> doctorRef = doctor.get();
			ManagedReference<CreatureObject*> patientRef = patient.get();

			if (sessionRef == nullptr || doctorRef == nullptr || patientRef == nullptr)
				return;

			// Validate zones exist (prevent crash if logged out during delay)
			if (doctorRef->getZone() == nullptr || patientRef->getZone() == nullptr)
				return;

			Locker doctorLocker(doctorRef);
			Locker patientLocker(patientRef, doctorRef);

			if (petIds.isEmpty() || attributes.isEmpty())
				return;

			doctorRef->removePendingTask(taskKey);

			bool doctorAvailable = isCreatureAvailableForAutomation(doctorRef);
			bool patientAvailable = isCreatureAvailableForAutomation(patientRef);

			if (!doctorAvailable || !patientAvailable) {
				if (!patientAvailable) {
					sessionRef->revokeService(patientRef, MedicalServiceSession::SERVICE_PET, false);

					if (doctorAvailable && doctorRef != nullptr) {
						StringBuffer docMsg;
						docMsg << patientRef->getFirstName() << " is no longer available for pet enhancements.";
						doctorRef->sendSystemMessage(docMsg.toString());
					}
				}

				return;
			}

			if (!sessionRef->isActive())
				return;

			if (!sessionRef->patientHasPetAccess(patientRef, false))
				return;

			int requiredMind = doctorRef->calculateCostAdjustment(CreatureAttribute::FOCUS, 150);

			if (doctorRef->getHAM(CreatureAttribute::MIND) < requiredMind) {
				if (!lowMindNotified) {
					patientRef->sendSystemMessage(LOW_MIND_MESSAGE_PATIENT);
					doctorRef->sendSystemMessage(LOW_MIND_MESSAGE_DOCTOR);
					lowMindNotified = true;
				}

				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_MIND_RETRY_DELAY);
				return;
			}

			lowMindNotified = false;

			// Validate indices
			if (petIndex < 0 || petIndex >= petIds.size()) {
				sessionRef->completePetBuff(patientRef, true);
				return;
			}

			ZoneServer* zoneServer = doctorRef->getZoneServer();

			if (zoneServer == nullptr)
				return;

			uint64 petId = petIds.get(petIndex);
			ManagedReference<SceneObject*> petObject = zoneServer->getObject(petId);

			// Validate pet object exists before casting
			if (petObject == nullptr) {
				petIndex++;
				attributeIndex = 0;
				waitCount = 0;
				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
				return;
			}

			ManagedReference<AiAgent*> petAgent = petObject.castTo<AiAgent*>();

			if (petAgent == nullptr || petAgent->isDroidObject() || petAgent->getZone() == nullptr) {
				petIndex++;
				attributeIndex = 0;
				waitCount = 0;
				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
				return;
			}

			ManagedReference<CreatureObject*> petCreature = petAgent.castTo<CreatureObject*>();

			if (petCreature == nullptr) {
				petIndex++;
				attributeIndex = 0;
				waitCount = 0;
				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
				return;
			}

			if (petCreature->isDead()) {
				petIndex++;
				attributeIndex = 0;
				waitCount = 0;
				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
				return;
			}

			float distance = doctorRef->getDistanceTo(petCreature);

			if (distance > DEFAULT_RANGE_METERS) {
				if (++waitCount > AUTO_SERVICE_MAX_WAIT_ATTEMPTS) {
					StringBuffer patientMsg;
					patientMsg << petCreature->getDisplayedName() << " moved out of range before the pet buff could complete.";
					patientRef->sendSystemMessage(patientMsg.toString());

					StringBuffer docMsg;
					docMsg << petCreature->getDisplayedName() << " moved out of range before the pet buff could complete.";
					doctorRef->sendSystemMessage(docMsg.toString());

					sessionRef->revokeService(patientRef, MedicalServiceSession::SERVICE_PET, true);
					return;
				}

				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_RANGE_RETRY_DELAY);
				return;
			}

			if (!doctorRef->canTreatWounds()) {
				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_COOLDOWN_RETRY_DELAY);
				return;
			}

			// Validate attribute index bounds
			if (attributeIndex < 0 || attributeIndex >= attributes.size()) {
				petIndex++;
				attributeIndex = 0;
				waitCount = 0;

				if (petIndex >= petIds.size()) {
					sessionRef->completePetBuff(patientRef, true);
					return;
				}

				doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
				return;
			}

			String attribute = attributes.get(attributeIndex);
			doctorRef->executeObjectControllerAction(STRING_HASHCODE("healenhance"), petCreature->getObjectID(), UnicodeString(attribute));

			attributeIndex++;
			waitCount = 0;

			doctorRef->addPendingTask(taskKey, this, AUTO_SERVICE_NEXT_STEP_DELAY);
		}
	};
}

void MedicalServiceSessionImplementation::initializeState(CreatureObject* practitioner, bool hospital) {
	this->practitioner = practitioner;
	hospitalMode = hospital;
	includeResists = true;
	active = false;
	enhancementPrice = 0;
	woundPrice = 0;
	rangeMeters = DEFAULT_RANGE_METERS;
	ticketDurationMs = DEFAULT_TICKET_DURATION_MS;

	activeTickets.removeAll();
	ticketExpiry.removeAll();

	activeTickets.setNullValue(0);
	ticketExpiry.setNullValue(0);
}

void MedicalServiceSessionImplementation::openSession(int enhancementPrice, int woundPrice, bool includeResists) {
	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	this->enhancementPrice = std::max(0, enhancementPrice);
	this->woundPrice = std::max(0, woundPrice);
	this->includeResists = includeResists;
	active = true;

	Locker locker(doctor);

	Reference<PlayerObject*> ghost = doctor->getPlayerObject();

	// Note: PlayerBitmasks::HEALING and BUFFING not available in this repository
	// Remote medical support methods not available in this repository

	StringBuffer msg;

	if (hospitalMode) {
		msg << "You open your hospital services.";
	} else {
		msg << "You open your clinic services.";
	}

	doctor->sendSystemMessage(msg.toString());
}

void MedicalServiceSessionImplementation::closeSession(bool notifyPractitioner) {
	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	Locker locker(doctor);

	active = false;
	activeTickets.removeAll();
	ticketExpiry.removeAll();

	doctor->clearState(CreatureState::IMMOBILIZED, true);

	Reference<PlayerObject*> ghost = doctor->getPlayerObject();

	// Note: PlayerBitmasks::HEALING and BUFFING not available in this repository
	// Remote medical support methods not available in this repository

	if (notifyPractitioner) {
		if (hospitalMode)
			doctor->sendSystemMessage("You close your hospital services.");
		else
			doctor->sendSystemMessage("You close your clinic services.");
	}
}

void MedicalServiceSessionImplementation::updateMode(bool hospital) {
	hospitalMode = hospital;

	if (!active)
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	Locker locker(doctor);

	Reference<PlayerObject*> ghost = doctor->getPlayerObject();

	// Note: PlayerBitmasks::BUFFING not available in this repository
}

void MedicalServiceSessionImplementation::updatePrices(int enhancementPrice, int woundPrice) {
	this->enhancementPrice = std::max(0, enhancementPrice);
	this->woundPrice = std::max(0, woundPrice);
}

void MedicalServiceSessionImplementation::updateIncludeResists(bool includeResists) {
	this->includeResists = includeResists;
}

bool MedicalServiceSessionImplementation::isHospitalMode() {
	return hospitalMode;
}

bool MedicalServiceSessionImplementation::canProvideEnhancements() {
	return hospitalMode;
}

bool MedicalServiceSessionImplementation::canProvideWoundHealing() {
	return true;
}

bool MedicalServiceSessionImplementation::includesResists() {
	return includeResists;
}

bool MedicalServiceSessionImplementation::isActive() {
	return active;
}

int MedicalServiceSessionImplementation::getEnhancementPrice() {
	return enhancementPrice;
}

int MedicalServiceSessionImplementation::getWoundPrice() {
	return woundPrice;
}

void MedicalServiceSessionImplementation::beginEnhancementRequest(CreatureObject* patient) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == doctor)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (patientHasEnhancementAccess(patient, false)) {
		patient->sendSystemMessage("You already have an active enhancement session with this practitioner.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to request enhancements.");
		return;
	}

	if (enhancementPrice <= 0) {
		Vector<String> attributeList;
		buildEnhancementAttributeList(attributeList, false);

		if (!hasEnhancementSupplies(doctor, attributeList)) {
			sendSupplyFailureMessages(doctor, patient,
					SUPPLY_FAILURE_PATIENT_MESSAGE,
					"You do not have the required enhancement packs to provide enhancements.");
			return;
		}

		grantServiceAccess(patient, SERVICE_ENHANCEMENT, true);
		return;
	}

	Reference<PlayerObject*> ghost = patient->getPlayerObject();

	if (ghost == nullptr)
		return;

	StringBuffer prompt;
	prompt << doctor->getFirstName() << " offers full enhancement services for " << enhancementPrice << " credits.";

	ManagedReference<SuiMessageBox*> box = new SuiMessageBox(patient, SuiWindowType::NONE);
	box->setPromptTitle("Hospital Service");
	box->setPromptText(prompt.toString());
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");
	box->setCallback(new MedicalServiceConsentSuiCallback(doctor->getZoneServer(), doctor, MedicalServiceConsentSuiCallback::ENHANCEMENTS));
	box->setUsingObject(doctor);

	ghost->addSuiBox(box);
	patient->sendMessage(box->generateMessage());
}

void MedicalServiceSessionImplementation::beginEnhancementResistRequest(CreatureObject* patient) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	if (!includesResists()) {
		patient->sendSystemMessage("This practitioner cannot provide resistance buffs.");
		return;
	}

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == doctor)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (patientHasEnhancementAccess(patient, false)) {
		patient->sendSystemMessage("You already have an active enhancement session with this practitioner.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to request enhancements.");
		return;
	}

	Vector<String> attributeList;
	buildEnhancementAttributeList(attributeList, true);

	if (!hasEnhancementSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required enhancement packs to provide enhancements.");
		return;
	}

	if (enhancementPrice <= 0) {
		grantServiceAccess(patient, SERVICE_ENHANCEMENT | SERVICE_ENHANCEMENT_RESIST, true);
		return;
	}

	Reference<PlayerObject*> ghost = patient->getPlayerObject();

	if (ghost == nullptr)
		return;

	StringBuffer prompt;
	prompt << doctor->getFirstName() << " offers enhancement services with resistances for " << enhancementPrice << " credits.";

	ManagedReference<SuiMessageBox*> box = new SuiMessageBox(patient, SuiWindowType::NONE);
	box->setPromptTitle("Hospital Service");
	box->setPromptText(prompt.toString());
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");
	box->setCallback(new MedicalServiceConsentSuiCallback(doctor->getZoneServer(), doctor, MedicalServiceConsentSuiCallback::ENHANCEMENTS_WITH_RESISTS));
	box->setUsingObject(doctor);

	ghost->addSuiBox(box);
	patient->sendMessage(box->generateMessage());
}

void MedicalServiceSessionImplementation::beginEntertainerBuffRequest(CreatureObject* patient) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == doctor)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (patientHasEnhancementAccess(patient, false)) {
		patient->sendSystemMessage("You already have an active enhancement session with this practitioner.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to request enhancements.");
		return;
	}

	Vector<String> attributeList;
	buildEntertainerAttributeList(attributeList);

	if (enhancementPrice <= 0) {
		if (!hasEnhancementSupplies(doctor, attributeList)) {
			sendSupplyFailureMessages(doctor, patient,
					SUPPLY_FAILURE_PATIENT_MESSAGE,
					"You do not have the required enhancement packs to provide enhancements.");
			return;
		}

		grantServiceAccess(patient, SERVICE_ENHANCEMENT | SERVICE_ENTERTAINER, true);
		return;
	}

	Reference<PlayerObject*> ghost = patient->getPlayerObject();

	if (ghost == nullptr)
		return;

	StringBuffer prompt;
	prompt << doctor->getFirstName() << " offers entertainer action buffs for " << enhancementPrice << " credits.";

	ManagedReference<SuiMessageBox*> box = new SuiMessageBox(patient, SuiWindowType::NONE);
	box->setPromptTitle("Entertainer Buff Service");
	box->setPromptText(prompt.toString());
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");
	box->setCallback(new MedicalServiceConsentSuiCallback(doctor->getZoneServer(), doctor, MedicalServiceConsentSuiCallback::ENTERTAINER));
	box->setUsingObject(doctor);

	ghost->addSuiBox(box);
	patient->sendMessage(box->generateMessage());
}

void MedicalServiceSessionImplementation::beginPetBuffRequest(CreatureObject* patient) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == doctor)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (patientHasPetAccess(patient, false)) {
		patient->sendSystemMessage("You already have an active pet buff session with this practitioner.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to request pet buffs.");
		return;
	}

	Vector<uint64> petIds;
	if (!collectOrganicPetIds(patient, petIds)) {
		patient->sendSystemMessage("You do not have any eligible pets active for buffing.");
		return;
	}

	Vector<String> attributeList;
	buildEnhancementAttributeList(attributeList, false);

	if (!hasEnhancementSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required enhancement packs to provide enhancements.");
		return;
	}

	if (enhancementPrice <= 0) {
		grantServiceAccess(patient, SERVICE_PET, true);
		return;
	}

	Reference<PlayerObject*> ghost = patient->getPlayerObject();

	if (ghost == nullptr)
		return;

	StringBuffer prompt;
	prompt << doctor->getFirstName() << " offers pet buff services for " << enhancementPrice << " credits.";

	ManagedReference<SuiMessageBox*> box = new SuiMessageBox(patient, SuiWindowType::NONE);
	box->setPromptTitle("Pet Buff Service");
	box->setPromptText(prompt.toString());
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");
	box->setCallback(new MedicalServiceConsentSuiCallback(doctor->getZoneServer(), doctor, MedicalServiceConsentSuiCallback::PET));
	box->setUsingObject(doctor);

	ghost->addSuiBox(box);
	patient->sendMessage(box->generateMessage());
}

void MedicalServiceSessionImplementation::beginWoundRequest(CreatureObject* patient) {
	if (patient == nullptr || !active || !canProvideWoundHealing())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == doctor)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (patientHasWoundAccess(patient, false)) {
		patient->sendSystemMessage("You already have an active wound session with this practitioner.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to request wound healing.");
		return;
	}

	if (woundPrice <= 0) {
		Vector<String> attributeList;
		buildWoundAttributeList(attributeList);

		if (!hasWoundSupplies(doctor, attributeList)) {
			sendSupplyFailureMessages(doctor, patient,
					SUPPLY_FAILURE_PATIENT_MESSAGE,
					"You do not have the required wound treatment packs to provide that service.");
			return;
		}

		grantServiceAccess(patient, SERVICE_WOUND, true);
		return;
	}

	Reference<PlayerObject*> ghost = patient->getPlayerObject();

	if (ghost == nullptr)
		return;

	StringBuffer prompt;
	prompt << doctor->getFirstName() << " offers wound treatment services for " << woundPrice << " credits.";

	ManagedReference<SuiMessageBox*> box = new SuiMessageBox(patient, SuiWindowType::NONE);
	box->setPromptTitle("Clinic Service");
	box->setPromptText(prompt.toString());
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");
	box->setCallback(new MedicalServiceConsentSuiCallback(doctor->getZoneServer(), doctor, MedicalServiceConsentSuiCallback::WOUND));
	box->setUsingObject(doctor);

	ghost->addSuiBox(box);
	patient->sendMessage(box->generateMessage());
}

void MedicalServiceSessionImplementation::confirmEnhancementRequest(CreatureObject* patient, bool accepted) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (!accepted) {
		patient->sendSystemMessage("You decline the enhancement offer.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to accept the enhancement offer.");
		return;
	}

	Vector<String> attributeList;
	buildEnhancementAttributeList(attributeList, false);

	if (!hasEnhancementSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required enhancement packs to provide enhancements.");
		return;
	}

	if (!collectPayment(patient, enhancementPrice))
		return;

	grantServiceAccess(patient, SERVICE_ENHANCEMENT, true);
}

void MedicalServiceSessionImplementation::confirmEnhancementResistRequest(CreatureObject* patient, bool accepted) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	if (!includesResists()) {
		patient->sendSystemMessage("This practitioner cannot provide resistance buffs.");
		return;
	}

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (!accepted) {
		patient->sendSystemMessage("You decline the enhancement offer.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to accept the enhancement offer.");
		return;
	}

	Vector<String> attributeList;
	buildEnhancementAttributeList(attributeList, true);

	if (!hasEnhancementSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required enhancement packs to provide enhancements.");
		return;
	}

	if (!collectPayment(patient, enhancementPrice))
		return;

	grantServiceAccess(patient, SERVICE_ENHANCEMENT | SERVICE_ENHANCEMENT_RESIST, true);
}

void MedicalServiceSessionImplementation::confirmEntertainerBuffRequest(CreatureObject* patient, bool accepted) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (!accepted) {
		patient->sendSystemMessage("You decline the entertainer buff offer.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to accept the entertainer buff offer.");
		return;
	}

	Vector<String> attributeList;
	buildEntertainerAttributeList(attributeList);

	if (!hasEnhancementSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required enhancement packs to provide enhancements.");
		return;
	}

	if (!collectPayment(patient, enhancementPrice))
		return;

	grantServiceAccess(patient, SERVICE_ENHANCEMENT | SERVICE_ENTERTAINER, true);
}

void MedicalServiceSessionImplementation::confirmPetBuffRequest(CreatureObject* patient, bool accepted) {
	if (patient == nullptr || !active || !canProvideEnhancements())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (!accepted) {
		patient->sendSystemMessage("You decline the pet buff offer.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to accept the pet buff offer.");
		return;
	}

	Vector<uint64> petIds;
	if (!collectOrganicPetIds(patient, petIds)) {
		patient->sendSystemMessage("You do not have any eligible pets active for buffing.");
		return;
	}

	Vector<String> attributeList;
	buildEnhancementAttributeList(attributeList, false);

	if (!hasEnhancementSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required enhancement packs to provide enhancements.");
		return;
	}

	if (!collectPayment(patient, enhancementPrice))
		return;

	grantServiceAccess(patient, SERVICE_PET, true);
}

void MedicalServiceSessionImplementation::confirmWoundRequest(CreatureObject* patient, bool accepted) {
	if (patient == nullptr || !active || !canProvideWoundHealing())
		return;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr)
		return;

	clearExpiredTickets(patient->getObjectID());

	if (!accepted) {
		patient->sendSystemMessage("You decline the wound treatment offer.");
		return;
	}

	if (!isWithinServiceRange(patient)) {
		patient->sendSystemMessage("You are too far away to accept the wound treatment offer.");
		return;
	}

	Vector<String> attributeList;
	buildWoundAttributeList(attributeList);

	if (!hasWoundSupplies(doctor, attributeList)) {
		sendSupplyFailureMessages(doctor, patient,
				SUPPLY_FAILURE_PATIENT_MESSAGE,
				"You do not have the required wound treatment packs to provide that service.");
		return;
	}

	if (!collectPayment(patient, woundPrice))
		return;

	grantServiceAccess(patient, SERVICE_WOUND, true);
}

bool MedicalServiceSessionImplementation::patientHasEnhancementAccess(CreatureObject* patient, bool notifyIfMissing) {
	return hasService(patient, SERVICE_ENHANCEMENT, notifyIfMissing);
}

bool MedicalServiceSessionImplementation::patientHasWoundAccess(CreatureObject* patient, bool notifyIfMissing) {
	return hasService(patient, SERVICE_WOUND, notifyIfMissing);
}

bool MedicalServiceSessionImplementation::patientHasPetAccess(CreatureObject* patient, bool notifyIfMissing) {
	return hasService(patient, SERVICE_PET, notifyIfMissing);
}

void MedicalServiceSessionImplementation::completeEnhancement(CreatureObject* patient, bool notifyPatient) {
	revokeService(patient, SERVICE_ENHANCEMENT | SERVICE_ENTERTAINER | SERVICE_ENHANCEMENT_RESIST, notifyPatient);
}

void MedicalServiceSessionImplementation::completePetBuff(CreatureObject* patient, bool notifyPatient) {
	revokeService(patient, SERVICE_PET, notifyPatient);
}

void MedicalServiceSessionImplementation::completeWound(CreatureObject* patient, bool notifyPatient) {
	revokeService(patient, SERVICE_WOUND, notifyPatient);
}

void MedicalServiceSessionImplementation::revokeAllAccess(CreatureObject* patient, bool notifyPatient) {
	if (patient == nullptr)
		return;

	uint64 id = patient->getObjectID();
	activeTickets.drop(id);
	ticketExpiry.drop(id);

	if (notifyPatient)
		patient->sendSystemMessage("Your medical session has been closed.");
}

void MedicalServiceSessionImplementation::grantServiceAccess(CreatureObject* patient, int serviceFlag, bool notify) {
	if (patient == nullptr)
		return;

	uint64 id = patient->getObjectID();

	int currentFlags = activeTickets.get(id);
	bool hadEnhancement = (currentFlags & SERVICE_ENHANCEMENT) != 0;
	bool hadWound = (currentFlags & SERVICE_WOUND) != 0;
	bool hadPet = (currentFlags & SERVICE_PET) != 0;

	currentFlags |= serviceFlag;
	activeTickets.put(id, currentFlags);
	ticketExpiry.put(id, System::getMiliTime() + ticketDurationMs);

	bool includesEnhancement = (serviceFlag & SERVICE_ENHANCEMENT) != 0;
	bool includesWound = (serviceFlag & SERVICE_WOUND) != 0;
	bool includesEntertainer = (serviceFlag & SERVICE_ENTERTAINER) != 0;
	bool includesResist = (serviceFlag & SERVICE_ENHANCEMENT_RESIST) != 0;
	bool includesPet = (serviceFlag & SERVICE_PET) != 0;

	bool newlyGrantedEnhancement = includesEnhancement && !hadEnhancement;
	bool newlyGrantedWound = includesWound && !hadWound;
	bool newlyGrantedPet = includesPet && !hadPet;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (notify) {
		String doctorName = doctor != nullptr ? doctor->getFirstName() : String("your practitioner");
		if (includesEnhancement && includesWound)
			patient->sendSystemMessage("You are now queued for enhancements and wound treatment with " + doctorName + ".");
		else if (includesEnhancement) {
			if (includesEntertainer)
				patient->sendSystemMessage("You are now queued for entertainer buffs with " + doctorName + ".");
			else if (includesResist)
				patient->sendSystemMessage("You are now queued for enhancements with resistances with " + doctorName + ".");
			else
				patient->sendSystemMessage("You are now queued for enhancements with " + doctorName + ".");
		} else if (includesWound) {
			patient->sendSystemMessage("You are now queued for wound treatment with " + doctorName + ".");
		}

		if (includesPet)
			patient->sendSystemMessage("You are now queued for pet buffs with " + doctorName + ".");

		if (doctor != nullptr) {
			StringBuffer docMsg;

			docMsg << patient->getFirstName() << " has paid for ";

			bool appended = false;
			if (includesEnhancement && includesWound) {
				docMsg << "enhancements and wound treatment";
				appended = true;
			} else if (includesEnhancement) {
				if (includesEntertainer)
					docMsg << "entertainer buffs";
				else if (includesResist)
					docMsg << "enhancements with resistances";
				else
					docMsg << "enhancements";
				appended = true;
			} else if (includesWound) {
				docMsg << "wound treatment";
				appended = true;
			}

			if (includesPet) {
				if (appended)
					docMsg << " and pet buffs";
				else
					docMsg << "pet buffs";
				appended = true;
			}

			if (!appended)
				docMsg << "services";

			docMsg << ".";
			doctor->sendSystemMessage(docMsg.toString());
		}
	}

	if (doctor == nullptr || doctor == patient || (!newlyGrantedEnhancement && !newlyGrantedWound && !newlyGrantedPet))
		return;

	if (!isCreatureAvailableForAutomation(doctor) || !isCreatureAvailableForAutomation(patient))
		return;

	ManagedReference<MedicalServiceSession*> sessionRef = _this.getReferenceUnsafeStaticCast();

	if (sessionRef == nullptr || !isPlayerCreature(patient))
		return;

	if (newlyGrantedEnhancement && sessionRef->canProvideEnhancements()) {
		String taskKey = buildAutoTaskKey(SERVICE_ENHANCEMENT, id);
		doctor->removePendingTask(taskKey);

		Vector<String> enhancementAttributes;

		if (includesEntertainer)
			buildEntertainerAttributeList(enhancementAttributes);
		else
			buildEnhancementAttributeList(enhancementAttributes, includesResist);

		int enhancementServiceFlag = SERVICE_ENHANCEMENT;

		if (includesEntertainer)
			enhancementServiceFlag |= SERVICE_ENTERTAINER;

		if (includesResist)
			enhancementServiceFlag |= SERVICE_ENHANCEMENT_RESIST;

		Reference<AutoMedicalServiceTask*> task = new AutoMedicalServiceTask(sessionRef.get(), doctor, patient,
				enhancementServiceFlag, false, taskKey, &enhancementAttributes);
		doctor->addPendingTask(taskKey, task, 0);
	}

	if (newlyGrantedWound && sessionRef->canProvideWoundHealing()) {
		String taskKey = buildAutoTaskKey(SERVICE_WOUND, id);
		doctor->removePendingTask(taskKey);

		Vector<String> woundAttributes;
		buildWoundAttributeList(woundAttributes);

		Reference<AutoMedicalServiceTask*> task = new AutoMedicalServiceTask(sessionRef.get(), doctor, patient,
				SERVICE_WOUND, false, taskKey, &woundAttributes);
		doctor->addPendingTask(taskKey, task, 0);
	}

	if (newlyGrantedPet && sessionRef->canProvideEnhancements()) {
		Vector<uint64> petIds;
		if (!collectOrganicPetIds(patient, petIds)) {
			patient->sendSystemMessage("You do not have any eligible pets active for buffing.");
			if (doctor != nullptr) {
				StringBuffer docWarn;
				docWarn << patient->getFirstName() << " does not have any eligible pets for buffing.";
				doctor->sendSystemMessage(docWarn.toString());
			}
			sessionRef->revokeService(patient, SERVICE_PET, false);
			return;
		}

		String taskKey = buildAutoTaskKey(SERVICE_PET, id);
		doctor->removePendingTask(taskKey);

		Reference<AutoPetEnhancementTask*> task = new AutoPetEnhancementTask(sessionRef.get(), doctor, patient, petIds, taskKey);
		doctor->addPendingTask(taskKey, task, 0);
	}
}

bool MedicalServiceSessionImplementation::collectPayment(CreatureObject* patient, int amount) {
	if (amount <= 0)
		return true;

	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == nullptr)
		return false;

	int totalFunds = patient->getCashCredits() + patient->getBankCredits();

	if (totalFunds < amount) {
		patient->sendSystemMessage("You do not have enough credits to pay for this service.");
		return false;
	}

	TransactionLog trx(patient, doctor, TrxCode::PLAYERMISCACTION, amount, true);

	int remaining = amount;
	int cash = patient->getCashCredits();
	int cashDebit = std::min(cash, remaining);

	if (cashDebit > 0) {
		patient->subtractCashCredits(cashDebit);
		doctor->addCashCredits(cashDebit, true);
		remaining -= cashDebit;
	}

	if (remaining > 0) {
		patient->subtractBankCredits(remaining);
		doctor->addBankCredits(remaining, true);
	}

	StringBuffer msg;
	msg << "You pay " << amount << " credits for medical services.";
	patient->sendSystemMessage(msg.toString());

	StringBuffer docMsg;
	docMsg << patient->getFirstName() << " has paid you " << amount << " credits.";
	doctor->sendSystemMessage(docMsg.toString());

	return true;
}

bool MedicalServiceSessionImplementation::hasService(CreatureObject* patient, int serviceFlag, bool notifyIfMissing) {
	if (patient == nullptr)
		return false;

	clearExpiredTickets(patient->getObjectID());

	uint64 id = patient->getObjectID();
	int flags = activeTickets.get(id);

	if ((flags & serviceFlag) != 0)
		return true;

	if (notifyIfMissing) {
		String missingMessage;

		if ((serviceFlag & SERVICE_PET) != 0)
			missingMessage = "You have not paid for pet buffs.";
		else if ((serviceFlag & SERVICE_ENHANCEMENT) != 0) {
			if ((serviceFlag & SERVICE_ENTERTAINER) != 0)
				missingMessage = "You have not paid for entertainer buffs.";
			else if ((serviceFlag & SERVICE_ENHANCEMENT_RESIST) != 0)
				missingMessage = "You have not paid for enhancements with resistances.";
			else
				missingMessage = "You have not paid for enhancements.";
		} else
			missingMessage = "You have not paid for wound treatment.";

		patient->sendSystemMessage(missingMessage);

		ManagedReference<CreatureObject*> doctor = practitioner.get();

		if (doctor != nullptr) {
			StringBuffer docMessage;
			docMessage << patient->getFirstName() << " has not paid for ";

			if ((serviceFlag & SERVICE_PET) != 0)
				docMessage << "pet buffs.";
			else if ((serviceFlag & SERVICE_ENHANCEMENT) != 0) {
				if ((serviceFlag & SERVICE_ENTERTAINER) != 0)
					docMessage << "entertainer buffs.";
				else if ((serviceFlag & SERVICE_ENHANCEMENT_RESIST) != 0)
					docMessage << "enhancements with resistances.";
				else
					docMessage << "enhancements.";
			} else
				docMessage << "wound treatment.";

			doctor->sendSystemMessage(docMessage.toString());
		}
	}

	return false;
}

void MedicalServiceSessionImplementation::revokeService(CreatureObject* patient, int serviceFlag, bool notifyPatient) {
	if (patient == nullptr)
		return;

	uint64 id = patient->getObjectID();
	int flags = activeTickets.get(id);

	if ((flags & serviceFlag) == 0)
		return;

	flags &= ~serviceFlag;

	if (flags == 0) {
		activeTickets.drop(id);
		ticketExpiry.drop(id);
	} else {
		activeTickets.put(id, flags);
	}

	if (notifyPatient) {
		if ((serviceFlag & SERVICE_PET) != 0) {
			patient->sendSystemMessage("Your pet buff session has been completed.");
		} else if ((serviceFlag & SERVICE_ENHANCEMENT) != 0) {
			if ((serviceFlag & SERVICE_ENTERTAINER) != 0)
				patient->sendSystemMessage("Your entertainer buff session has been completed.");
			else if ((serviceFlag & SERVICE_ENHANCEMENT_RESIST) != 0)
				patient->sendSystemMessage("Your enhancement session with resistances has been completed.");
			else
				patient->sendSystemMessage("Your enhancement session has been completed.");
		} else {
			patient->sendSystemMessage("Your wound treatment session has been completed.");
		}
	}
}

void MedicalServiceSessionImplementation::clearExpiredTickets(uint64 patientId) {
	if (!ticketExpiry.contains(patientId))
		return;

	long expiry = ticketExpiry.get(patientId);

	if (expiry == 0)
		return;

	if (System::getMiliTime() > (uint64) expiry) {
		ticketExpiry.drop(patientId);
		activeTickets.drop(patientId);
	}
}

bool MedicalServiceSessionImplementation::isWithinServiceRange(CreatureObject* patient) {
	ManagedReference<CreatureObject*> doctor = practitioner.get();

	if (doctor == nullptr || patient == nullptr)
		return false;

	float distance = doctor->getDistanceTo(patient);
	return distance <= (float) rangeMeters;
}
