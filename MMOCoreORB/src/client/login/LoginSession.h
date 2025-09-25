/*
 * LoginSession.h
 *
 *  Created on: Sep 6, 2009
 *      Author: theanswer
 */

#ifndef LOGINSESSION_H_
#define LOGINSESSION_H_

#include "system/lang.h"
#include "engine/log/Logger.h"
#include "server/login/objects/GalaxyList.h"
#include "server/login/objects/CharacterListEntry.h"

class LoginClient;

class LoginSession : public Mutex, public Runnable, public Logger, public Object {
	Condition sessionFinalized;

	uint32 accountID;
	String sessionID;

	Vector<CharacterListEntry> characters;
	int selectedCharacter;

	int instance;

	class LoginClientThread* loginThread;

	Reference<LoginClient*> login;

	VectorMap<uint32, Galaxy> galaxies;

public:
	LoginSession(int instance);

	~LoginSession();

	void run();

	void addCharacter(uint64 objectID) {
		CharacterListEntry entry;
		entry.setObjectID(objectID);
		characters.add(entry);
	}

	void addCharacter(uint64 objectID, uint32 galaxyID) {
		CharacterListEntry entry;
		entry.setObjectID(objectID);
		entry.setGalaxyID(galaxyID);
		characters.add(entry);
	}

	void addCharacter(const CharacterListEntry& entry) {
		characters.add(entry);
	}

	void setSelectedCharacter(int id) {
		lock();
		selectedCharacter = id;
		sessionFinalized.signal(this);
		unlock();
	}

	void signalCompletion() {
		lock();
		sessionFinalized.signal(this);
		unlock();
	}

	void setAccountID(uint32 id) {
		accountID = id;
	}

	void setSessionID(const String& sessionID) {
		this->sessionID = sessionID;
	}

	uint32 getAccountID() {
		return accountID;
	}

	const String& getSessionID() {
		return sessionID;
	}

	int getSelectedCharacter() {
		return selectedCharacter;
	}

	const CharacterListEntry& getCharacter(uint32 id) {
		return characters.get(id);
	}

	void addGalaxy(const Galaxy& galaxy) {
		galaxies.put(galaxy.getID(), galaxy);
	}

	Galaxy* getGalaxyInfo(uint32 galaxyId) {
		if (galaxies.contains(galaxyId)) {
			return &galaxies.get(galaxyId);
		}
		return nullptr;
	}
};


#endif /* LOGINSESSION_H_ */
