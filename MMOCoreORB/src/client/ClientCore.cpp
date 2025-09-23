/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include "client/zone/Zone.h"
#include "client/zone/managers/object/ObjectManager.h"

#include "ClientCore.h"

#include "client/login/LoginSession.h"

ClientCore::ClientCore(int instances) : Core("log/core3client.log", "client3"), Logger("CoreClient") {
	ClientCore::instances = instances;

	setInfoLogLevel();
}

void ClientCore::initialize() {
	info("starting up client..");
}

int connectCount = 0, disconnectCount = 0;

void ClientCore::run() {
	for (int i = 0; i < instances; ++i) {
		zones.add(nullptr);
	}

	info("initialized", true);

	loginCharacter(0);

	System::out << "Waiting for zone connection..." << endl;

	// Wait for zone to be fully loaded
	bool zoneReady = false;
	int attempts = 0;
	const int maxAttempts = 60; // 30 seconds max wait
	
	while (!zoneReady && attempts < maxAttempts) {
		Thread::sleep(500);
		attempts++;
		
		Zone* zone = zones.get(0);
		if (zone != nullptr && zone->isSceneLoaded()) {
			zoneReady = true;
			System::out << "Zone connection established and scene loaded!" << endl;
		}
	}
	
	if (!zoneReady) {
		System::out << "Timeout waiting for zone connection" << endl;
	}

	System::out << "Shutting down..." << endl;

	for (int i = 0; i < instances; ++i) {
		Zone* zone = zones.get(i);
		if (zone != nullptr)
			zone->disconnect();
	}
}

void ClientCore::loginCharacter(int index) {
	try {
		Zone* zone = zones.get(index);
		if (zone != nullptr)
			return;

		Reference<LoginSession*> loginSession = new LoginSession(index);
		loginSession->run();

		uint32 selectedCharacter = loginSession->getSelectedCharacter();
		uint64 objid = 0;

		if (selectedCharacter != -1) {
			objid = loginSession->getCharacterObjectID(selectedCharacter);
			info("trying to login " + String::valueOf(objid));
		}

		uint32 acc = loginSession->getAccountID();
		const String& sessionID = loginSession->getSessionID();

		System::out << "Login completed - Account: " << acc << ", Session: " << sessionID << endl;

		// Enable zone connection
		zone = new Zone(index, objid, acc, sessionID);
		zone->start();
		zones.set(index, zone);

		connectCount++;
	} catch (Exception& e) {
		e.printMessage();
	}
}

void ClientCore::logoutCharacter(int index) {
	Zone* zone = zones.get(index);
	if (zone == nullptr || !zone->isStarted())
		return;

	zones.set(index, nullptr);

	zone->disconnect();

	disconnectCount++;

	delete zone;
}

void ClientCore::handleCommands() {
	// Disabled for now
}

int main(int argc, char* argv[]) {
	try {
		Vector<String> arguments;
		for (int i = 1; i < argc; ++i) {
			arguments.add(argv[i]);
		}

		StackTrace::setBinaryName("core3client");

		int instances = 1;

		if (argc > 1)
			instances = Integer::valueOf(arguments.get(0));

		ClientCore core(instances);

		core.start();
	} catch (Exception& e) {
		System::out << e.getMessage() << "\n";
		e.printStackTrace();
	}

	return 0;
}
