#include "SRStructureObject.h"

#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/ZoneServer.h"

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

void SRStructureObject::clearPackedItems() {
	Locker locker(&lock);
	packedCellItems.clear();
}

void SRStructureObject::collectItems(BuildingObject* building) {
	if (building == nullptr)
		return;
	Locker locker(&lock);
	packedCellItems.clear();
	int total = building->getTotalCellNumber();
	for (int i = 1; i <= total; ++i) {
		CellObject* cell = building->getCell(i);
		if (cell == nullptr)
			continue;
		std::vector<uint64> oids;
		int size = cell->getContainerObjectsSize();
		for (int j = 0; j < size; ++j) {
			auto obj = cell->getContainerObject(j);
			if (obj != nullptr && !obj->isCreatureObject()) {
				oids.push_back(obj->getObjectID());
			}
		}
		if (!oids.empty())
			packedCellItems.emplace(i, std::move(oids));
	}
}

void SRStructureObject::restoreItems(BuildingObject* building, ZoneServer* zoneServer) {
	if (building == nullptr || zoneServer == nullptr)
		return;
	Locker locker(&lock);
	for (auto& pair : packedCellItems) {
		int cellNum = pair.first;
		CellObject* cell = building->getCell(cellNum);
		if (cell == nullptr)
			continue;
		for (uint64 oid : pair.second) {
			auto obj = zoneServer->getObject(oid);
			if (obj == nullptr)
				continue;
			Locker objLocker(obj);
			if (obj->getZone() == nullptr) {
				// Reinsert into the cell only if it no longer resides in a zone
				cell->transferObject(obj, -1, true);
			}
		}
	}
}

void to_json(nlohmann::json& j, const SRStructureObject& p) {
}