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
		System::out << "Unhandled packet - opcount: " << opcount << " opcode: 0x" << hex << opcode << dec << endl;
		break;
	}
}

void LoginPacketHandler::handleErrorMessage(Message* pack) {
	String errorType, errorMessage;

	pack->parseAscii(errorType);
	pack->parseAscii(errorMessage);

	System::out << "ERROR: " << errorType << " - " << errorMessage << endl;

	loginSession->signalCompletion();
}

void LoginPacketHandler::handleLoginClientToken(Message* pack) {
	System::out << "Received login token" << endl;

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

	System::out << "Account ID: " << accountID << endl;
	System::out << "Session ID: " << sessionID << endl;
	System::out << "Station ID: " << stationID << endl;
	System::out << "Username: " << username << endl;
}

void LoginPacketHandler::handleLoginEnumCluster(Message* pack) {
	System::out << "\n=== GALAXIES (Basic Info) ===" << endl;

	uint32 galaxyCount = pack->parseInt();
	System::out << "Galaxy Count: " << galaxyCount << endl;

	for (int i = 0; i < galaxyCount; ++i) {
		uint32 galaxyID = pack->parseInt();

		String galaxyName;
		pack->parseAscii(galaxyName);

		uint32 serverStatus = pack->parseInt();
		uint32 footer = pack->parseInt();

		System::out << "Galaxy[" << i << "]("
			<< "id: " << galaxyID
			<< ", name: " << galaxyName
			<< ", serverStatus: 0x" << hex << serverStatus << dec
			<< ", footer: 0x" << hex << footer << dec
			<< ")" << endl;
	}
}

void LoginPacketHandler::handleLoginClusterStatus(Message* pack) {
	System::out << "\n=== GALAXY STATUS (Full Info) ===" << endl;

	uint32 galaxyCount = pack->parseInt();
	System::out << "Galaxy Count: " << galaxyCount << endl;

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

		System::out << "Galaxy[" << i << "]("
			<< "id: " << galaxyID
			<< ", address: " << address
			<< ", port: " << port
			<< ", pingPort: " << pingPort
			<< ", population: " << population
			<< ", maxCapacity: 0x" << hex << maxCapacity << dec
			<< ", distance: " << distance
			<< ", status: 0x" << hex << status << dec
			<< ", recommended: " << recommended
			<< ", unknown: 0x" << hex << (int)unknown << dec
			<< ")" << endl;
	}
}

void LoginPacketHandler::handleEnumerateCharacterId(Message* pack) {
	System::out << "\n=== CHARACTERS ===" << endl;

	uint32 characters = pack->parseInt();
	System::out << "Character Count: " << characters << endl;

	if (loginSession == nullptr)
		return;

	if (characters == 0) {
		System::out << "No characters found" << endl;
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

		System::out << "Character[" << i << "]("
			<< "name: " << name.toString()
			<< ", oid: " << oid
			<< ", galaxy: " << galaxy
			<< ", crc: 0x" << hex << crc << dec
			<< ", serverStatus: 0x" << hex << serverStatus << dec
			<< ")" << endl;

		loginSession->addCharacter(oid);
	}

	// Now that all characters are added, select the first one and signal completion
	loginSession->setSelectedCharacter(0);
}
