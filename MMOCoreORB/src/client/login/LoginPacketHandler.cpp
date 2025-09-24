/*
 * LoginPacketHandler.cpp
 *
 *  Created on: Sep 4, 2009
 *      Author: theanswer
 */

#include "LoginSession.h"

#include "LoginPacketHandler.h"

void LoginPacketHandler::handleMessage(Message* pack) {
	sys::uint16 opcount = pack->parseShort();
	sys::uint32 opcode = pack->parseInt();

	switch (opcount) {
	case 02:
		switch (opcode) {
		case STRING_HASHCODE("EnumerateCharacterId"): // 0x65EA4574 char create success
			handleEnumerateCharacterId(pack);
			break;
		case STRING_HASHCODE("LoginEnumCluster"): // 0xC11C63B9 galaxy list
			handleLoginEnumCluster(pack);
			break;
		}
		break;
	case 03:
		switch (opcode) {
		case STRING_HASHCODE("ErrorMessage"):
			handleErrorMessage(pack);
			break;
		case STRING_HASHCODE("LoginClusterStatus"): // 0x3436AEB6 galaxy status with address/port
			handleLoginClusterStatus(pack);
			break;
		}
		break;
	case 04:
		switch (opcode) {
		case STRING_HASHCODE("LoginClientToken"): // 0xAAB296C6 client token
			handleLoginClientToken(pack);
			break;
		}
		break;

	default:
		error() << "Unhandled packet - opcount: " << opcount << " opcode: 0x" << hex << opcode << dec;
		break;
	}
}

void LoginPacketHandler::handleErrorMessage(Message* pack) {
	String errorType, errorMessage;

	pack->parseAscii(errorType);
	pack->parseAscii(errorMessage);

	info(true) << "ERROR: " << errorType << " - " << errorMessage ;

	loginSession->signalCompletion();
}

void LoginPacketHandler::handleLoginClientToken(Message* pack) {
	info(true) << "Received login token" ;

	uint32 sessionLength = pack->parseInt();
	uint32 len = sessionLength - 4;

	String sessionID;
	for (uint32 i = 0; i < len; ++i) {
		char byte = pack->parseByte();
		sessionID += byte;
	}

	uint32 accountID = pack->parseInt();
	uint32 stationID = pack->parseInt();

	String username;
	pack->parseAscii(username);

	loginSession->setAccountID(accountID);
	loginSession->setSessionID(sessionID);

	info(true) << "Account ID: " << accountID ;
	info(true) << "Session ID: " << sessionID ;
	info(true) << "Station ID: " << stationID ;
	info(true) << "Username: " << username ;
}

void LoginPacketHandler::handleLoginEnumCluster(Message* pack) {
	info(true) << "=== GALAXIES (Basic Info) ===" ;

	uint32 galaxyCount = pack->parseInt();
	info(true) << "Galaxy Count: " << galaxyCount ;

	for (int i = 0; i < galaxyCount; ++i) {
		uint32 galaxyID = pack->parseInt();

		String galaxyName;
		pack->parseAscii(galaxyName);

		uint32 serverStatus = pack->parseInt();
		uint32 footer = pack->parseInt();

		// Create Galaxy with ID and name
		Galaxy galaxy(galaxyID, galaxyName);
		loginSession->addGalaxy(galaxy);

		info(true) << "Galaxy[" << i << "]: " << galaxy;
	}
}

void LoginPacketHandler::handleLoginClusterStatus(Message* pack) {
	info(true) << "=== GALAXY STATUS (Full Info) ===" ;

	uint32 galaxyCount = pack->parseInt();
	info(true) << "Galaxy Count: " << galaxyCount ;

	for (int i = 0; i < galaxyCount; ++i) {
		uint32 galaxyID = pack->parseInt();

		String address;
		pack->parseAscii(address);

		uint16 port = pack->parseShort();
		uint16 pingPort = pack->parseShort();

		uint32 population = pack->parseInt();
		uint32 maxCapacity = pack->parseInt();
		uint32 distance = pack->parseInt();
		uint32 status = pack->parseInt();
		uint32 recommended = pack->parseInt();
		uint8 unknown = pack->parseByte();

		// Update existing galaxy with connection details
		Galaxy* galaxy = loginSession->getGalaxyInfo(galaxyID);
		if (galaxy != nullptr) {
			galaxy->updateClusterStatus(address, port, pingPort, population);
			info(true) << "Galaxy[" << i << "]: " << *galaxy;
		} else {
			info(true) << "WARNING: Galaxy " << galaxyID << " not found for cluster status update";
		}
	}
}

void LoginPacketHandler::handleEnumerateCharacterId(Message* pack) {
	info(true) << "=== CHARACTERS ===" ;

	uint32 characters = pack->parseInt();
	info(true) << "Character Count: " << characters ;

	if (loginSession == nullptr)
		return;

	if (characters == 0) {
		info(true) << "No characters found" ;
		loginSession->setSelectedCharacter(-1);
		loginSession->signalCompletion();
		return;
	}

	// First, add all characters to the vector
	for (int i = 0; i < characters; ++i) {
		UnicodeString name;
		pack->parseUnicode(name);

		uint32 crc = pack->parseInt();
		uint64 oid = pack->parseLong();
		uint32 galaxy = pack->parseInt();
		uint32 serverStatus = pack->parseInt();

		CharacterListEntry entry;
		entry.setObjectID(oid);
		entry.setGalaxyID(galaxy);
		entry.setFirstName(name.toString());

		loginSession->addCharacter(entry);

		info(true) << "Character[" << i << "]: " << entry;
	}

	// Now that all characters are added, select the first one and signal completion
	loginSession->setSelectedCharacter(0);
}
