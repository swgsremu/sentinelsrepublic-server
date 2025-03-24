#include "SRStructureObject.h"

SRStructureObject::SRStructureObject()
	: controlDevice(nullptr) {
}

SRStructureObject::~SRStructureObject() {
}

SRStructureObject::SRStructureObject(const SRStructureObject& spl)
	: controlDevice(spl.controlDevice) {
}

SRStructureObject& SRStructureObject::operator=(const SRStructureObject& list) {
	if (this == &list) {
		return *this;
	}
	controlDevice = list.controlDevice;
	return *this;
}

bool SRStructureObject::toBinaryStream(ObjectOutputStream* stream) {
	return true; // Placeholder: assume success
}

bool SRStructureObject::parseFromBinaryStream(ObjectInputStream* stream) {
	return true; // Placeholder: assume success
}


String SRStructureObject::getPackupMessage() {
	return "";
}

void SRStructureObject::setControlDevice(ControlDevice* device) {
	Locker locker(&lock);
	controlDevice = device;
}

ControlDevice* SRStructureObject::getControlDevice() const {
	return controlDevice;
}

bool SRStructureObject::isPackedUp() const {
	return controlDevice != nullptr;
};

bool SRStructureObject::unloadFromZone(bool sendSelfDestroy) {
	return true;
}

void to_json(nlohmann::json& j, const SRStructureObject& p) {
}