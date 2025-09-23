#include "Zone.h"
#include "ZoneClientThread.h"

#include "server/zone/packets/zone/ClientIdMessage.h"
#include "client/zone/managers/objectcontroller/ObjectController.h"
#include "client/zone/managers/object/ObjectManager.h"

int Zone::createdChar = 0;

Zone::Zone(int instance, uint64 characterObjectID, uint32 account, const String& sessionID) : Thread(), Mutex("Zone") {
	characterID = characterObjectID;
	accountID = account;
	this->sessionID = sessionID;
	player = nullptr;

	objectManager = new ObjectManager();
	objectManager->setZone(this);

	objectController = new ObjectController(this);

	client = nullptr;
	clientThread = nullptr;

	Zone::instance = instance;
	started = false;
	sceneLoaded = false;

	System::out << "Zone created for character " << characterObjectID << " with sessionID: " << sessionID << endl;
}

Zone::~Zone() {
	delete objectManager;
	objectManager = nullptr;

	clientThread->stop();
}

void Zone::run() {
	try {
		System::out << "Zone::run() starting..." << endl;

		client = new ZoneClient(44463);
		client->setAccountID(accountID);
		client->setZone(this);
		client->getClient()->setLoggingName("ZoneClient" + String::valueOf(instance));
		client->initialize();

		System::out << "ZoneClient created and initialized" << endl;

		clientThread = new ZoneClientThread(client);
		clientThread->start();

		System::out << "ZoneClientThread started" << endl;

		if (client->connect()) {
			System::out << "Connected to zone server" << endl;
		} else {
			System::out << "ERROR: Could not connect to zone server" << endl;
			return;
		}

		startTime.updateToCurrentTime();

		// Send ClientIdMessage with sessionID
		System::out << "Sending ClientIdMessage..." << endl;
		BaseMessage* acc = new ClientIdMessage(accountID, sessionID);
		client->sendMessage(acc);
		System::out << "ClientIdMessage sent" << endl;

		started = true;

#ifdef WITH_STM
	//TransactionalMemoryManager::commitPureTransaction();
#endif

	} catch (sys::lang::Exception& e) {
		System::out << "Zone::run() exception: " << e.getMessage() << "\n";
		exit(0);
	}
}

void Zone::insertPlayer() {
	if (player == nullptr)
		return;

	insertPlayer(player);
}

void Zone::insertPlayer(PlayerCreature* player) {
	if (player == nullptr)
		return;

	playerArray.add(player);
}

SceneObject* Zone::getObject(uint64 objid) {
	return objectManager->getObject(objid);
}

PlayerCreature* Zone::getSelfPlayer() {
	return (PlayerCreature*)objectManager->getObject(characterID);
}

void Zone::disconnect() {
	if (client != nullptr) {
		client->disconnect();
	}
}

void Zone::sceneStarted() {
	System::out << "Zone started in " << startTime.miliDifference() << "ms" << endl;
	sceneLoaded = true;
}

void Zone::follow(const String& name) {
	SceneObject* object = objectManager->getObject(name);

	if (object == nullptr) {
		System::out << "ERROR: " << name << " not found" << endl;
		return;
	}

	PlayerCreature* player = getSelfPlayer();

	Locker _locker(player);
	player->setFollow(object);

	System::out << "Started following " << name << endl;
}

void Zone::stopFollow() {
	PlayerCreature* player = getSelfPlayer();

	Locker _locker(player);

	player->setFollow(nullptr);
	System::out << "Stopped following" << endl;
}

void Zone::lurk() {
	PlayerCreature* player = getSelfPlayer();

	if (player == nullptr)
		return;

	Locker _locker(player);

	// Remove lurking functionality for now - PlayerCreature doesn't have these methods
	System::out << "Lurking not implemented" << endl;
}

bool Zone::doCommand(const String& command, const String& arguments) {
	// ObjectController expects uint32 crc, not String - disable for now
	System::out << "Command: " << command << " Args: " << arguments << endl;
	return true;
}
