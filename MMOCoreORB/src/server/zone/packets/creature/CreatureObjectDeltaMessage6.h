/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef CREATUREOBJECTDELTAMESSAGE6_H_
#define CREATUREOBJECTDELTAMESSAGE6_H_

#include "server/zone/packets/tangible/TangibleObjectDeltaMessage6.h"
#include "server/zone/objects/creature/CreatureObject.h"

class CreatureObjectDeltaMessage6 : public TangibleObjectDeltaMessage6 {
	CreatureObject* creo;

public:
	CreatureObjectDeltaMessage6(CreatureObject* cr) : TangibleObjectDeltaMessage6(cr, 0x4352454F) {
		creo = cr;
	}

	void updateLevel(uint16 value) {
		startUpdate(0x02);
		insertShort(value);
	}

	void updatePerformanceAnimation(const String& pAnimation) {
		startUpdate(0x03);
		insertAscii(pAnimation);
	}

	void updateMoodStr() {
		addAsciiUpdate(0x04, creo->getMoodString());
	}

	void updateWeapon() {
		addLongUpdate(0x05, creo->getWeaponID());
	}

	void updateGroupID() {
		addLongUpdate(0x06, creo->getGroupID());
	}

	void updateInviterId() {
		startUpdate(0x07);

		insertLong(creo->getGroupInviterID());
		insertLong(creo->getGroupInviteCounter());
	}

	void updateGuildID() {
		addIntUpdate(0x08, creo->getGuildID());
	}

	void updateTarget() {
		addLongUpdate(0x09, creo->getTargetID());
	}

	void updateMoodID() {
		addByteUpdate(0x0A, creo->getMoodID());
	}

	void updatePerformanceStartTime(uint32 startTime) {
		startUpdate(0x0B);
		insertInt(startTime);
	}

	void updatePerformanceType(int value) {
		startUpdate(0x0C);
		insertInt(value);
	}

	void updateAlternateAppearance() {
		startUpdate(0x10);
		insertAscii(creo->getAlternateAppearance());
	}
};

#endif /*CREATUREOBJECTDELTAMESSAGE6_H_*/
