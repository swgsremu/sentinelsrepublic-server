#include "Zone.h"
#include "ZoneClientThread.h"

#include "server/zone/packets/zone/ClientIdMessage.h"
#include "client/zone/managers/objectcontroller/ObjectController.h"
#include "client/zone/managers/object/ObjectManager.h"

int Zone::createdChar = 0;

Zone::Zone(int instance, uint64 characterObjectID, uint32 account, const String& sessionID, const String& galaxyAddress, uint32 galaxyPort) : Thread(), Mutex("Zone"), Logger("Zone") {
	characterID = characterObjectID;
	accountID = account;
	this->sessionID = sessionID;
	this->galaxyAddress = galaxyAddress;
	this->galaxyPort = galaxyPort;
	player = nullptr;

	objectManager = new ObjectManager();
	objectManager->setZone(this);

	objectController = new ObjectController(this);

	client = nullptr;
	clientThread = nullptr;

	Zone::instance = instance;
	started = false;
	sceneLoaded = false;

	info(true) << "Zone created for character " << characterObjectID << " with sessionID: " << sessionID;
}

Zone::~Zone() {
	delete objectManager;
	objectManager = nullptr;
}

void Zone::run() {
	try {
		info(true) << "Zone::run() starting...";

		client = new ZoneClient(galaxyAddress, galaxyPort);
		client->setAccountID(accountID);
		client->setZone(this);
		client->getClient()->setLoggingName("ZoneClient" + String::valueOf(instance));
		client->initialize();

		info(true) << "ZoneClient created and initialized";

		clientThread = new ZoneClientThread(client);
		clientThread->start();

		info(true) << "ZoneClientThread started";

		if (client->connect()) {
			info(true) << "Connected to zone server";
		} else {
			error() << "ERROR: Could not connect to zone server";
			return;
		}

		startTime.updateToCurrentTime();

		// Send ClientIdMessage with sessionID
		info(true) << "Sending ClientIdMessage...";
		BaseMessage* acc = new ClientIdMessage(accountID, sessionID);
		client->sendMessage(acc);
		info(true) << "ClientIdMessage sent";

		started = true;

#ifdef WITH_STM
	//TransactionalMemoryManager::commitPureTransaction();
#endif

	} catch (sys::lang::Exception& e) {
		error() << "Zone::run() exception: " << e.getMessage();
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
	if (objectManager == nullptr)
		return nullptr;

	return objectManager->getObject(objid);
}

PlayerCreature* Zone::getSelfPlayer() {
	return (PlayerCreature*)objectManager->getObject(characterID);
}

void Zone::disconnect() {
	if (client != nullptr) {
		client->disconnect();
		// Don't call clientThread->stop() - disconnect() already stops the client
		// and the thread will exit naturally when the client stops
	}
}

void Zone::sceneStarted() {
	info(true) << "Zone started in " << startTime.miliDifference() << "ms";
	sceneLoaded = true;
}

void Zone::follow(const String& name) {
	SceneObject* object = objectManager->getObject(name);

	if (object == nullptr) {
		info(true) << "ERROR: " << name << " not found";
		return;
	}

	PlayerCreature* player = getSelfPlayer();

	Locker _locker(player);
	player->setFollow(object);

	info(true) << "Started following " << name;
}

void Zone::stopFollow() {
	PlayerCreature* player = getSelfPlayer();

	Locker _locker(player);

	player->setFollow(nullptr);
	info(true) << "Stopped following";
}

void Zone::lurk() {
	PlayerCreature* player = getSelfPlayer();

	if (player == nullptr)
		return;

	Locker _locker(player);

	// Remove lurking functionality for now - PlayerCreature doesn't have these methods
	info(true) << "Lurking not implemented";
}

bool Zone::doCommand(const String& command, const String& arguments) {
	// ObjectController expects uint32 crc, not String - disable for now
	info(true) << "Command: " << command << " Args: " << arguments;
	return true;
}
