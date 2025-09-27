/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include "client/zone/Zone.h"
#include "client/zone/managers/object/ObjectManager.h"

#include "ClientCore.h"

#include "client/login/LoginSession.h"

int exit_result = 1;

ClientCore::ClientCore(const String& username, const String& password) : Core("log/core3client.log", "client3"), Logger("CoreClient") {
	ClientCore::username = username;
	ClientCore::password = password;

	Core::initializeProperties("Client3");
	setLogLevel(static_cast<LogLevel>(Core::getIntProperty("Client3.LogLevel", 1)));
}

void ClientCore::initialize() {
	info(true) << __PRETTY_FUNCTION__ << " start";
}

void ClientCore::run() {
	info(true) << "initialized";

	if (!loginCharacter()) {
		info(true) << "Failed to login character.";
		exit_result = 101;
		return;
	}

	info(true) << "Waiting for zone connection...";

	if (zone != nullptr && zone->waitForSceneReady(Core::getIntProperty("Client3.ZoneTimeout", 30) * 1000)) {
		info(true) << "Zone connection established and scene loaded!";
		info(true) << "Login flow test completed successfully!";
	} else {
		error() << "Timeout waiting for zone connection";
		exit_result = 102;
	}

	info(true) << "Shutting down...";

	logoutCharacter();

	exit_result = 0;
}

bool ClientCore::loginCharacter() {
	try {
		info(true) << "Logging in as: " << username;

		Reference<LoginSession*> loginSession = new LoginSession(username, password);
		loginSession->run();

		auto numCharacters = loginSession->getCharacterListSize();

		if (numCharacters == 0) {
			info(true) << __FUNCTION__ << ": No characters found to login?";
			return false;
		}

		uint32 selectedCharacter = System::random(numCharacters - 1);

		const CharacterListEntry& character = loginSession->getCharacterByIndex(selectedCharacter);
		auto objid = character.getObjectID();
		auto galaxyId = character.getGalaxyID();

		info(true) << "Login[" << selectedCharacter << "]: " << character;

		uint32 acc = loginSession->getAccountID();
		const String& sessionID = loginSession->getSessionID();

		info(true) << "Login completed - Account: " << acc << ", Session: " << sessionID;

		auto galaxy = loginSession->getGalaxy(galaxyId);

		info(true) << "Zone into " << galaxy;

		if (galaxy.getAddress().isEmpty()) {
			throw Exception("Invalid galaxy, missing IP address.");
		}

		loginSession = nullptr;

		zone = new Zone(objid, acc, sessionID, galaxy.getAddress(), galaxy.getPort());
		zone->start();
	} catch (Exception& e) {
		e.printMessage();
		return false;
	}

	return true;
}

void ClientCore::logoutCharacter() {
	if (zone == nullptr || !zone->isStarted())
		return;

	info(true) << __FUNCTION__ << "(" << index << ")";

	zone->disconnect();

	delete zone;
}

int main(int argc, char* argv[]) {
	try {
		String username, password;

		// Get credentials from environment variables
		const char* envUser = getenv("CORE3_CLIENT_USERNAME");
		const char* envPass = getenv("CORE3_CLIENT_PASSWORD");

		if (envUser && envPass) {
			username = envUser;
			password = envPass;
		} else {
			throw Exception("ERROR: Please set CORE3_CLIENT_USERNAME and CORE3_CLIENT_PASSWORD environment variables");
		}

		StackTrace::setBinaryName("core3client");

		// Configure engine3
		Core::setProperty("TaskManager.defaultSchedulerThreads", "2");
		Core::setProperty("TaskManager.defaultIOSchedulers", "2");
		Core::setProperty("TaskManager.defaultWorkerQueues", "1");
		Core::setProperty("TaskManager.defaultWorkerThreadsPerQueue", "2");

		ClientCore core(username, password);
		core.start();
		System::out << "core.start() returned" << endl;
	} catch (Exception& e) {
		System::out << e.getMessage() << "\n";
		e.printStackTrace();
		exit_result = 100;
	}

	System::out << "exit(" << exit_result << ")" << endl;

	return exit_result;
}
