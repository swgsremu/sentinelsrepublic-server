#ifndef SHIPDELTAVECTOR_H_
#define SHIPDELTAVECTOR_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/packets/DeltaMessage.h"

class ShipDeltaVector : public Object {
protected:
	VectorMap<uint32, DeltaMessage*> deltaVector;

	uint64 objectID;
	uint64 playerID;

	mutable ReadWriteLock deltaMutex;

	enum DeltaTypeID : uint32 {
		None	= 0,
		Delta1	= 1,
		Delta3	= 3,
		Delta4	= 4,
		Delta6	= 6,
	};

	enum DeltaType : uint32 {
		Private	= 1,
		Public	= 2,
	};

	int getDeltaType(uint32 deltaID) {
		switch (deltaID) {
			case DeltaTypeID::Delta1: return DeltaType::Private;
			case DeltaTypeID::Delta3: return DeltaType::Public;
			case DeltaTypeID::Delta4: return DeltaType::Private;
			case DeltaTypeID::Delta6: return DeltaType::Public;
		}

		return DeltaTypeID::None;
	}

public:
	ShipDeltaVector(SceneObject* ship, SceneObject * player = nullptr) : Object() {
		deltaVector.setNoDuplicateInsertPlan();
		deltaVector.setNullValue(nullptr);

		objectID = ship->getObjectID();
		playerID = player != nullptr ? player->getObjectID() : 0;
	}

	~ShipDeltaVector() {
		reset();
	}

	void reset(SceneObject* player = nullptr) {
		Locker lock(&deltaMutex);

		playerID = player != nullptr ? player->getObjectID() : 0;

		for (int i = deltaVector.size() - 1; i >= 0; --i) {
			auto message = deltaVector.elementAt(i).getValue();

			if (message != nullptr) {
				delete message;
				message = nullptr;
			}

			deltaVector.remove(i);
		}
	}

	DeltaMessage* getMessage(uint32 deltaID) {
		ReadLocker lock(&deltaMutex);

		int deltaType = getDeltaType(deltaID);
		if (deltaType == DeltaTypeID::None || (deltaType == DeltaType::Private && playerID == DeltaTypeID::None)) {
			return nullptr;
		}

		auto delta = deltaVector.get(deltaID);
		if (delta != nullptr) {
			return delta;
		}

		delta = new DeltaMessage(objectID, 'SHIP', deltaID);
		deltaVector.put(deltaID, delta);

		return delta;
	}

	void sendMessages(SceneObject* ship, SceneObject* player = nullptr) {
		Locker lock(&deltaMutex);

		for (int i = 0; i < deltaVector.size(); ++ i) {
			auto deltaID = deltaVector.elementAt(i).getKey();
			auto message = deltaVector.elementAt(i).getValue();

			if (message == nullptr) {
				continue;
			}

			message->close();

			int deltaType = getDeltaType(deltaID);

			if (deltaType == DeltaType::Private && player != nullptr) {
				player->sendMessage(message->clone());
			} else if (deltaType == DeltaType::Public) {
				ship->broadcastMessage(message->clone(), false);
			}
		}

		reset(player);
	}

	int size() {
		ReadLocker lock(&deltaMutex);

		return deltaVector.size();
	}
};

#endif // SHIPDELTAVECTOR_H_
