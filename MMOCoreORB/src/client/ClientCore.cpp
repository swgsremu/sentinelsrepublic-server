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

	info(true)<< "Waiting for zone connection...";

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
			info(true)<< "Zone connection established and scene loaded!";
		}
	}

	if (!zoneReady) {
		info(true)<< "Timeout waiting for zone connection";
	} else {
		info(true)<< "Login flow test completed successfully!";
	}

	info(true)<< "Shutting down...";

	for (int i = 0; i < instances; ++i) {
		logoutCharacter(i);
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

		info(true) << "Login completed - Account: " << acc << ", Session: " << sessionID;

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

	info(true) << __FUNCTION__ << "(" << index << ")";

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
