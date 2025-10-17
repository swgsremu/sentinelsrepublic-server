/*
 * ConnectToZoneAction.cpp
 *
 * Transitions from login phase to zone phase
 * Cleans up login connection and establishes zone connection
 */

#include "client/ActionBase.h"
#include "client/ActionResult.h"
#include "client/ActionManager.h"
#include "client/ClientCore.h"
#include "client/login/LoginSession.h"
#include "client/zone/Zone.h"
#include "server/login/objects/CharacterListEntry.h"

class ConnectToZoneAction : public ActionBase {
	ConnectToZoneResult result;
	bool skipped;

public:
	ConnectToZoneAction() : skipped(false) {
		setLogLevel(Logger::INFO);
		setLoggingName("ConnectToZone");
	}

	const char* getName() const override {
		return "connectToZone";
	}

	int parseArgs(int index, int argc, char** argv) override {
		// No CLI arguments - auto-inserted by framework
		return 0;
	}

	void parseJSON(const JSONSerializationType& config) override {
		// No JSON configuration needed
	}

	bool needsZone() const override {
		return false;  // This action CREATES the zone connection
	}

	void run(ClientCore& core) override {
		// Validate prerequisites
		if (core.loginSession == nullptr || !core.loginSession->isLoggedIn()) {
			result.setError("No active login session", 1);
			return;
		}

		// Cleanup login connection
		if (core.loginSession->isConnected()) {
			info() << "Cleaning up login connection...";
			core.loginSession->cleanup();
		}

		// Determine which character to zone into
		uint64 characterOid = 0;
		uint32 galaxyId = 0;
		auto numCharacters = core.loginSession->getCharacterListSize();

		if (numCharacters > 0) {
			// Select character based on options
			Optional<const CharacterListEntry&> character;

			if (core.options.characterOid != 0) {
				character = core.loginSession->selectCharacterByOID(core.options.characterOid);
				if (!character) {
					result.setError("Character OID not found in account", 2);
					return;
				}
			} else if (!core.options.characterFirstname.isEmpty()) {
				character = core.loginSession->selectCharacterByFirstname(core.options.characterFirstname);
				if (!character) {
					result.setError("Character firstname not found in account", 3);
					return;
				}
			} else {
				// Select random character
				character = core.loginSession->selectRandomCharacter();
			}

			if (!character) {
				result.setError("Failed to select any character", 4);
				return;
			}

			characterOid = character->getObjectID();
			galaxyId = character->getGalaxyID();

			info() << "Selected character: " << character->getFirstName() << " (OID: " << characterOid << ")";
		} else {
			// No characters - will create in zone (if createCharacter flag is set)
			if (!core.options.createCharacter) {
				result.setError("No characters on account and character creation not enabled", 5);
				return;
			}

			// Use first available galaxy
			auto& galaxyMap = core.loginSession->getGalaxies();
			if (galaxyMap.size() == 0) {
				result.setError("No galaxies available", 6);
				return;
			}
			galaxyId = galaxyMap.get(0).getID();
			info() << "No characters - will create new character on galaxy " << galaxyId;
		}

		// Get galaxy info
		auto& galaxy = core.loginSession->getGalaxy(galaxyId);
		if (galaxy.getAddress().isEmpty()) {
			result.setError("Invalid galaxy - missing IP address", 7);
			return;
		}

		info() << "Connecting to zone: " << galaxy.getName()
		       << " at " << galaxy.getAddress() << ":" << galaxy.getPort();

		// Create and start zone connection
		uint32 accountId = core.loginSession->getAccountID();
		const String& sessionId = core.loginSession->getSessionID();

		core.zone = new Zone(characterOid, accountId, sessionId, galaxy.getAddress(), galaxy.getPort());
		core.zone->start();

		// Wait for zone connection
		int zoneTimeout = ClientCore::getZoneTimeout() * 1000;
		if (!core.zone->waitForSceneReady(zoneTimeout)) {
			result.setError(
				core.zone->getLastError().isEmpty()
					? "Zone connection timeout"
					: core.zone->getLastError(),
				core.zone->getLastErrorCode() != 0
					? core.zone->getLastErrorCode()
					: 8
			);
			return;
		}

		// Success
		result.setSuccess();
		result.setZoneInfo(galaxy.getAddress(), galaxy.getPort(), characterOid);

		info() << "Zone connection established successfully";
	}

	bool isOK() const override {
		return !skipped && result.isOK();
	}

	String getError() const override {
		return result.getError();
	}

	uint16 getErrorCode() const override {
		return result.getErrorCode();
	}

	void setSkipped() override {
		skipped = true;
	}

	JSONSerializationType toJSON() const override {
		JSONSerializationType json;
		json["action"] = getName();

		if (skipped) {
			json["status"] = "skipped";
		} else if (result.isOK()) {
			json["status"] = "ok";
			json["galaxyAddress"] = result.getGalaxyAddress().toCharArray();
			json["galaxyPort"] = result.getGalaxyPort();
			json["characterOid"] = result.getCharacterOid();
		} else {
			json["status"] = "failed";
			json["error"] = result.getError().toCharArray();
			json["errorCode"] = result.getErrorCode();
		}

		return json;
	}

	String getHelpText() const override {
		return "connectToZone: Connect to zone server (auto-inserted before zone actions)";
	}

	// Factory function for static registration
	static ActionBase* factory() {
		return new ConnectToZoneAction();
	}
};

// Static registration (runs before main())
static bool _registered_connectToZone =
	(ActionManager::registerAction("connectToZone", ConnectToZoneAction::factory), true);
