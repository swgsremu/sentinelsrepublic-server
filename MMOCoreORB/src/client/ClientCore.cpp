/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include <boost/program_options.hpp>
#include <fstream>
#include "engine/util/JSONSerializationType.h"
#include "client/zone/Zone.h"
#include "client/zone/managers/object/ObjectManager.h"

#include "ClientCore.h"

#include "client/login/LoginSession.h"
#include "client/ActionBase.h"
#include "client/ActionManager.h"
#include "server/zone/packets/charcreation/ClientCreateCharacter.h"

int exit_result = 1;

ClientCore::ClientCore(const ClientCoreOptions& opts) : Core("log/core3client.log", "client3"), Logger("CoreClient") {
	options = opts;
	zone = nullptr;

	overallStartTime.updateToCurrentTime();

	Core::initializeProperties("Client3");

	// Fill in missing values from client3.lua
	options.updateWithProperties();

	// Override with user-provided values (higher priority)
	options.saveToProperties();

	setLogLevel(static_cast<LogLevel>(options.logLevel));
	info(true) << "Log level set to: " << Logger::getLogType(static_cast<LogLevel>(options.logLevel)) << "(" << options.logLevel << ")";

	info(true) << "Current options: " << options;
}

void ClientCore::initialize() {
	info(true) << __PRETTY_FUNCTION__ << " start";
}

void ClientCore::parseJSONIntoActions(const JSONSerializationType& jsonActions, Vector<ActionBase*>& actions) {
	bool hasLoginAccount = false;
	bool hasConnectToZone = false;

	for (auto& actionConfig : jsonActions) {
		if (!actionConfig.contains("action")) {
			error() << "JSON action missing 'action' field";
			continue;
		}

		String actionName = String(actionConfig["action"].get<std::string>().c_str());
		ActionBase* action = ActionManager::createAction(actionName.toCharArray());

		if (action == nullptr) {
			error() << "Unknown action type: " << actionName;
			continue;
		}

		// Track connector actions
		if (actionName == "loginAccount") {
			hasLoginAccount = true;
		}
		if (actionName == "connectToZone") {
			hasConnectToZone = true;
		}

		// Auto-insert connectToZone before first zone action
		if (action->needsZone() && !hasConnectToZone) {
			ActionBase* connector = ActionManager::createAction("connectToZone");
			if (connector != nullptr) {
				actions.add(connector);
				hasConnectToZone = true;
				info(true) << "Auto-inserted connectToZone before " << actionName;
			}
		}

		// Parse action-specific config with variable substitution
		// Note: Variable substitution happens at string level for simplicity
		// Convert JSON to string, substitute, parse back
		String configStr = actionConfig.dump().c_str();
		String substituted = substituteVars(configStr);

		JSONSerializationType substitutedConfig;
		try {
			substitutedConfig = JSONSerializationType::parse(substituted.toCharArray());
		} catch (...) {
			// If parse fails, use original (no variables to substitute)
			substitutedConfig = actionConfig;
		}

		action->parseJSON(substitutedConfig);
		actions.add(action);
	}

	// Always ensure loginAccount at position 0
	if (!hasLoginAccount) {
		ActionBase* login = ActionManager::createAction("loginAccount");
		if (login != nullptr) {
			Vector<ActionBase*> temp;
			temp.add(login);
			for (int k = 0; k < actions.size(); k++) {
				temp.add(actions.get(k));
			}
			actions = temp;
			info(true) << "Auto-inserted loginAccount at start";
		}
	}
}

void ClientCore::parseArgumentsIntoActions(int argc, char** argv, Vector<ActionBase*>& actions) {
	bool hasLoginAccount = false;
	bool hasConnectToZone = false;

	for (int i = 1; i < argc; ) {
		int consumed = 0;

		// First: ClientCoreOptions gets first shot
		consumed = options.parseArgs(i, argc, argv);
		if (consumed > 0) {
			i += consumed;
			continue;
		}

		// Second: Try each registered action type
		Vector<String> actionNames = ActionManager::listActions();
		for (int j = 0; j < actionNames.size(); j++) {
			ActionBase* action = ActionManager::createAction(actionNames.get(j).toCharArray());
			if (action == nullptr) continue;

			consumed = action->parseArgs(i, argc, argv);

			if (consumed > 0) {
				// This action claimed the args

				// Track connector actions
				if (strcmp(action->getName(), "loginAccount") == 0) {
					hasLoginAccount = true;
				}
				if (strcmp(action->getName(), "connectToZone") == 0) {
					hasConnectToZone = true;
				}

				// Auto-insert connectToZone before first zone action
				if (action->needsZone() && !hasConnectToZone) {
					ActionBase* connector = ActionManager::createAction("connectToZone");
					if (connector != nullptr) {
						actions.add(connector);
						hasConnectToZone = true;
						info(true) << "Auto-inserted connectToZone before " << action->getName();
					}
				}

				actions.add(action);
				i += consumed;
				break;
			}

			delete action;  // Didn't want this arg
		}

		if (consumed == 0) {
			i++;
		}
	}

	// Always ensure loginAccount at position 0
	if (!hasLoginAccount) {
		ActionBase* login = ActionManager::createAction("loginAccount");
		if (login != nullptr) {
			// Vector doesn't have insert, so we need to prepend manually
			Vector<ActionBase*> temp;
			temp.add(login);
			for (int k = 0; k < actions.size(); k++) {
				temp.add(actions.get(k));
			}
			actions = temp;
			info(true) << "Auto-inserted loginAccount at start";
		}
	}

	// If --create-character was in options, add that action
	if (options.createCharacter && !hasConnectToZone) {
		ActionBase* create = ActionManager::createAction("createCharacter");
		if (create != nullptr) {
			// Need to auto-insert connectToZone first
			ActionBase* connector = ActionManager::createAction("connectToZone");
			if (connector != nullptr) {
				actions.add(connector);
				info(true) << "Auto-inserted connectToZone for character creation";
			}
			actions.add(create);
			info(true) << "Added createCharacter action from options";
		}
	}

	// If not login-only mode and no zone connection requested, add connectToZone
	// This maintains backward compatibility with old behavior
	if (!options.loginOnly && !hasConnectToZone) {
		ActionBase* connector = ActionManager::createAction("connectToZone");
		if (connector != nullptr) {
			actions.add(connector);
			info(true) << "Auto-inserted connectToZone (default behavior, not login-only)";
		}
	}
}

void ClientCore::executeActions() {
	for (int i = 0; i < actions.size(); i++) {
		ActionBase* action = actions.get(i);

		info(true) << "Running action: " << action->getName();
		action->run(*this);

		if (!action->isOK()) {
			error() << action->getName() << " failed: " << action->getError();
			exit_result = 100 + action->getErrorCode();

			// Mark remaining actions as skipped
			for (int j = i + 1; j < actions.size(); j++) {
				actions.get(j)->setSkipped();
			}

			break;
		}
	}
}

void ClientCore::run() {
	info(true) << "Core3Client starting";

	if (options.hasJSON() && options.getJSON().contains("actions")) {
		parseJSONIntoActions(options.getJSON()["actions"], actions);
	} else {
		parseArgumentsIntoActions(options.argc, options.argv, actions);
	}

	if (actions.size() == 0) {
		error() << "No actions to execute";
		exit_result = 100;
		return;
	}

	executeActions();

	if (exit_result == 1) {
		exit_result = 0;
	}

	if (options.waitAfterZone > 0 && zone != nullptr && zone->isSceneReady()) {
		info(true) << "Staying connected for " << options.waitAfterZone << " seconds...";
		Thread::sleep(options.waitAfterZone * 1000);
		info(true) << "Wait complete, proceeding to shutdown...";
	}

	if (!options.saveState.isEmpty()) {
		saveStateToFile(options.saveState, loginSession);
	}

	info(true) << "Shutting down...";

	if (zone != nullptr && zone->isStarted()) {
		logoutCharacter();
	}
	if (loginSession != nullptr && loginSession->isConnected()) {
		loginSession->cleanup();
	}

	for (int i = 0; i < actions.size(); i++) {
		delete actions.get(i);
	}
}

bool ClientCore::loginCharacter(Reference<LoginSession*>& loginSession) {
	try {
		info(true) << "Logging in as: " << options.username;

		loginSession = new LoginSession(options.username, options.password);
		loginSession->run();

		auto numCharacters = loginSession->getCharacterListSize();

		if (numCharacters == 0) {
			// Check if we should create a character
			if (options.createCharacter) {
				info(true) << __FUNCTION__ << ": No characters found - will attempt creation in zone";
				// Continue with zone connection with characterID=0
				// ZonePacketHandler will handle creation
			} else {
				info(true) << __FUNCTION__ << ": No characters found and character creation not enabled";
				return false;
			}
		}

		Optional<const CharacterListEntry&> character;
		uint64 objid = 0;
		uint32 galaxyId = 0;

		if (numCharacters > 0) {
			if (options.characterOid != 0) {
				character = loginSession->selectCharacterByOID(options.characterOid);
				if (!character) {
					info(true) << "ERROR: Character with OID " << options.characterOid << " not found";
					return false;
				}
			} else if (!options.characterFirstname.isEmpty()) {
				character = loginSession->selectCharacterByFirstname(options.characterFirstname);
				if (!character) {
					info(true) << "ERROR: Character with firstname '" << options.characterFirstname << "' not found";
					return false;
				}
			} else {
				character = loginSession->selectRandomCharacter();
			}

			if (!character) {
				info(true) << __FUNCTION__ << ": Failed to select any character";
				return false;
			}

			objid = character->getObjectID();
			galaxyId = character->getGalaxyID();

			info(true) << "Selected character: " << *character;
		} else {
			// No characters - will create in zone
			// Use first available galaxy
			auto galaxyList = loginSession->getGalaxies();
			if (galaxyList.size() == 0) {
				error() << "No galaxies available";
				return false;
			}
			galaxyId = galaxyList.get(0).getID();
			info(true) << "No character selected - will create new character";
		}

		uint32 acc = loginSession->getAccountID();
		const String& sessionID = loginSession->getSessionID();

		info(true) << "Login completed - Account: " << acc << ", Session: " << sessionID;

		auto galaxy = loginSession->getGalaxy(galaxyId);

		// Store selected galaxy for JSON output
		selectedGalaxy = galaxy;

		info(true) << "Zone into " << galaxy;

		if (galaxy.getAddress().isEmpty()) {
			throw Exception("Invalid galaxy, missing IP address.");
		}

		if (!options.loginOnly) {
			zone = new Zone(objid, acc, sessionID, galaxy.getAddress(), galaxy.getPort());
			zone->start();
		}
	} catch (Exception& e) {
		e.printMessage();
		return false;
	}

	return true;
}

BaseMessage* ClientCore::buildCreateCharacterPacket() {
	Core* instance = Core::getCoreInstance();
	if (!instance) {
		throw Exception("Cannot build character creation packet: No ClientCore instance");
	}

	ClientCore* clientCore = static_cast<ClientCore*>(instance);
	auto& opts = clientCore->options;

	// Generate default name if not provided
	String charName = opts.createCharName;
	if (charName.isEmpty()) {
		charName = opts.username + String::valueOf(System::random(9999));
	}

	// Default race if not provided
	String race = opts.createCharRace;
	if (race.isEmpty()) {
		race = "object/creature/player/human_male.iff";
	}

	// Default profession if not provided
	String profession = opts.createCharProfession;
	if (profession.isEmpty()) {
		profession = "crafting_artisan";
	}

	// Clamp height
	float height = opts.createCharHeight;
	if (height < 0.7f || height > 1.5f) {
		clientCore->warning("Height " + String::valueOf(height) + " out of range, clamping to 1.0");
		height = 1.0f;
	}

	UnicodeString unicodeName(charName.toCharArray());
	UnicodeString unicodeBio(opts.createCharBiography.toCharArray());

	clientCore->info(true) << "Creating character:";
	clientCore->info(true) << "  Name: " << charName;
	clientCore->info(true) << "  Race: " << race;
	clientCore->info(true) << "  Profession: " << profession;
	clientCore->info(true) << "  Height: " << height;
	clientCore->info(true) << "  Skip Tutorial: " << opts.createCharSkipTutorial;

	return new ClientCreateCharacter(
		unicodeName,
		race,
		height,
		opts.createCharCustomization,
		opts.createCharHairTemplate,
		opts.createCharHairCustomization,
		profession,
		unicodeBio,
		!opts.createCharSkipTutorial  // Note: inverted
	);
}

void ClientCore::logoutCharacter() {
	if (zone == nullptr || !zone->isStarted())
		return;

	info(true) << __FUNCTION__ << "(" << index << ")";

	// Disconnect from zone to stop receiving new packets
	zone->disconnect();

	// Wait for already-queued tasks to complete processing
	auto taskManager = Core::getTaskManager();
	int maxIterations = 20; // 20 x 50ms = 1 second max

	for (int i = 0; i < maxIterations; i++) {
		int executing = taskManager->getExecutingTaskSize();
		int scheduled = taskManager->getScheduledTaskSize();

		if (executing == 0 && scheduled == 0) {
			info(true) << "All tasks completed after " << (i * 50) << "ms";
			break;
		}

		Thread::sleep(50);
	}

	info(true) << "Processed " << zone->getZoneClient()->getPacketCount() << " total zone packets";

	delete zone;
}

void ClientCore::saveStateToFile(const String& filename, LoginSession* loginSession) {
	try {
		JSONSerializationType jsonData;

		// Overall timing
		jsonData["totalElapsedMs"] = overallStartTime.miliDifference();

		// Timestamp
		Time now;
		now.updateToCurrentTime();
		jsonData["@timestamp"] = now.getFormattedTimeFull().toCharArray();
		jsonData["time_msecs"] = now.getMiliTime();

		// Exit code
		jsonData["exitCode"] = exit_result;

		// Login stats
		if (loginSession != nullptr) {
			jsonData["loginStats"] = loginSession->collectStats();

			// Account info
			JSONSerializationType account;
			account["id"] = loginSession->getAccountID();
			account["username"] = loginSession->getSessionID().isEmpty() ? "" : loginSession->collectStats()["username"];
			account["sessionId"] = loginSession->getSessionID().toCharArray();
			jsonData["account"] = account;

			// Galaxies - iterate through all galaxies in the map
			JSONSerializationType galaxiesArray = JSONSerializationType::array();
			auto& galaxyMap = loginSession->getGalaxies();
			for (int i = 0; i < galaxyMap.size(); ++i) {
				auto& galaxy = galaxyMap.get(i);
				if (!galaxy.getName().isEmpty()) {
					JSONSerializationType galaxyObj;
					galaxyObj["id"] = galaxy.getID();
					galaxyObj["name"] = galaxy.getName().toCharArray();
					galaxyObj["address"] = galaxy.getAddress().toCharArray();
					galaxyObj["port"] = galaxy.getPort();
					galaxyObj["population"] = galaxy.getPopulation();
					galaxiesArray.push_back(galaxyObj);
				}
			}
			jsonData["galaxies"] = galaxiesArray;

			// Characters from login (may be empty if new account)
			JSONSerializationType charactersArray = JSONSerializationType::array();
			for (int i = 0; i < loginSession->getCharacterListSize(); ++i) {
				const auto& character = loginSession->getCharacterByIndex(i);
				JSONSerializationType charObj;
				charObj["name"] = character.getFirstName().toCharArray();
				charObj["oid"] = character.getObjectID();
				charObj["galaxyId"] = character.getGalaxyID();
				charObj["source"] = "existing";
				charactersArray.push_back(charObj);
			}

			// Add newly created character if applicable
			if (zone != nullptr && zone->isCharacterCreated()) {
				JSONSerializationType newCharObj;
				newCharObj["name"] = options.createCharName.toCharArray();
				newCharObj["oid"] = zone->getCreatedCharacterOID();
				newCharObj["galaxyId"] = selectedGalaxy ? selectedGalaxy->getID() : 0;
				newCharObj["source"] = "created";

				// Include creation parameters
				JSONSerializationType creationParams;
				creationParams["race"] = options.createCharRace.toCharArray();
				creationParams["profession"] = options.createCharProfession.toCharArray();
				creationParams["height"] = options.createCharHeight;
				creationParams["skipTutorial"] = options.createCharSkipTutorial;
				newCharObj["creationParams"] = creationParams;

				charactersArray.push_back(newCharObj);
			}

			jsonData["characters"] = charactersArray;
		}

		// Character creation state
		JSONSerializationType charCreation;
		charCreation["attempted"] = options.createCharacter;

		if (options.createCharacter) {
			if (zone != nullptr && zone->isCharacterCreated()) {
				charCreation["success"] = true;
				charCreation["characterOid"] = zone->getCreatedCharacterOID();
				charCreation["characterName"] = options.createCharName.toCharArray();

				// Echo back what was requested
				JSONSerializationType requestedParams;
				requestedParams["name"] = options.createCharName.toCharArray();
				requestedParams["race"] = options.createCharRace.isEmpty()
					? "object/creature/player/human_male.iff"
					: options.createCharRace.toCharArray();
				requestedParams["profession"] = options.createCharProfession.isEmpty()
					? "crafting_artisan"
					: options.createCharProfession.toCharArray();
				requestedParams["height"] = options.createCharHeight;
				requestedParams["customization"] = options.createCharCustomization.toCharArray();
				requestedParams["hairTemplate"] = options.createCharHairTemplate.toCharArray();
				requestedParams["hairCustomization"] = options.createCharHairCustomization.toCharArray();
				requestedParams["biography"] = options.createCharBiography.toCharArray();
				requestedParams["skipTutorial"] = options.createCharSkipTutorial;
				charCreation["requestedParams"] = requestedParams;

			} else if (zone != nullptr && zone->hasCharacterCreationFailed()) {
				charCreation["success"] = false;
				charCreation["error"] = "Character creation failed - see server errors";

				// Still include what was attempted
				JSONSerializationType attemptedParams;
				attemptedParams["name"] = options.createCharName.toCharArray();
				attemptedParams["race"] = options.createCharRace.isEmpty()
					? "object/creature/player/human_male.iff"
					: options.createCharRace.toCharArray();
				attemptedParams["profession"] = options.createCharProfession.isEmpty()
					? "crafting_artisan"
					: options.createCharProfession.toCharArray();
				attemptedParams["height"] = options.createCharHeight;
				charCreation["attemptedParams"] = attemptedParams;

			} else {
				charCreation["success"] = false;
				charCreation["error"] = "Character creation not completed (may have timed out or never reached zone server)";
			}
		} else {
			charCreation["success"] = nullptr;  // N/A
		}

		jsonData["characterCreation"] = charCreation;

		// Zone stats
		if (zone != nullptr) {
			jsonData["zoneStats"] = zone->collectStats();
			jsonData["zoneConnected"] = true;

			// Selected zone info
			JSONSerializationType selectedZone;
			selectedZone["characterId"] = zone->getCharacterID();
			selectedZone["address"] = zone->getGalaxyAddress().toCharArray();
			selectedZone["port"] = zone->getGalaxyPort();
			jsonData["selectedZone"] = selectedZone;
		} else {
			jsonData["zoneConnected"] = false;
		}

		// Success flags
		jsonData["loginSuccess"] = (loginSession != nullptr && loginSession->getAccountID() != 0);
		jsonData["overallSuccess"] = (exit_result == 0);
		jsonData["exitCode"] = exit_result;

		// Selected galaxy (if we got that far)
		if (selectedGalaxy) {
			JSONSerializationType selectedGalaxyJson;
			selectedGalaxyJson["id"] = selectedGalaxy->getID();
			selectedGalaxyJson["name"] = selectedGalaxy->getName().toCharArray();
			selectedGalaxyJson["address"] = selectedGalaxy->getAddress().toCharArray();
			selectedGalaxyJson["port"] = selectedGalaxy->getPort();
			selectedGalaxyJson["population"] = selectedGalaxy->getPopulation();
			jsonData["selectedGalaxy"] = selectedGalaxyJson;
		}

		// Runtime options
		jsonData["runtimeOptions"] = options.getAsJSON();

		// Actions array (new format)
		JSONSerializationType actionsArray = JSONSerializationType::array();
		for (int i = 0; i < actions.size(); i++) {
			actionsArray.push_back(actions.get(i)->toJSON());
		}
		jsonData["actions"] = actionsArray;

		// Variables map
		if (vars.size() > 0) {
			JSONSerializationType varsObj;
			for (int i = 0; i < vars.size(); i++) {
				varsObj[vars.elementAt(i).getKey().toCharArray()] = vars.elementAt(i).getValue().toCharArray();
			}
			jsonData["variables"] = varsObj;
		}

		// Write to file
		std::ofstream file(filename.toCharArray());
		if (file.is_open()) {
			file << jsonData.dump(2);  // Pretty print with 2-space indent
			file.close();
			info(true) << "State saved to: " << filename;
		} else {
			error() << "Failed to open file for writing: " << filename;
		}

	} catch (Exception& e) {
		error() << "Error saving state: " << e.getMessage();
	}
}

String ClientCoreOptions::resolveFileReference(const String& value) {
	// Check if value starts with '@'
	if (value.isEmpty() || value.charAt(0) != '@') {
		return value;
	}

	// Extract the file path (everything after '@')
	String filePath = value.subString(1);

	// Try to open and read the file
	std::ifstream file(filePath.toCharArray());
	if (!file.is_open()) {
		String errorMsg = "ERROR: Cannot read file referenced by '@': " + filePath;
		System::err << errorMsg << endl;
		throw Exception(errorMsg);
	}

	// Read the entire file content
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	// Convert to String and trim whitespace
	String result(content.c_str());
	return result.trim();
}

void ClientCoreOptions::loadEnvFile(const String& filename) {
	std::ifstream file(filename.toCharArray());
	if (!file.is_open()) return;

	std::string line;
	while (std::getline(file, line)) {
		String envLine(line.c_str());
		envLine = envLine.trim();
		if (envLine.isEmpty() || envLine.charAt(0) == '#') continue;

		StringTokenizer tokenizer(envLine);
		tokenizer.setDelimeter("=");

		if (tokenizer.hasMoreTokens()) {
			String key = tokenizer.getStringToken();
			if (tokenizer.hasMoreTokens()) {
				String value;
				tokenizer.finalToken(value);
				setenv(key.toCharArray(), value.toCharArray(), 1);
			}
		}
	}
}

bool ClientCoreOptions::loadFromJSON(const String& filename) {
	try {
		std::ifstream file(filename.toCharArray());
		if (!file.is_open()) {
			return false;
		}

		JSONSerializationType json;
		file >> json;

		// Save full JSON for action parsing
		jsonConfig = json;
		jsonLoaded = true;

		// Load all options from JSON
		if (json.contains("username")) username = json["username"];
		if (json.contains("password")) password = resolveFileReference(String(json["password"].get<std::string>().c_str()));
		if (json.contains("loginHost")) loginHost = json["loginHost"];
		if (json.contains("loginPort")) loginPort = json["loginPort"];
		if (json.contains("loginTimeout")) loginTimeout = json["loginTimeout"];
		if (json.contains("zoneTimeout")) zoneTimeout = json["zoneTimeout"];
		if (json.contains("clientVersion")) clientVersion = json["clientVersion"];
		if (json.contains("logLevel")) logLevel = json["logLevel"];
		if (json.contains("characterOid")) characterOid = json["characterOid"];
		if (json.contains("characterFirstname")) characterFirstname = json["characterFirstname"];
		if (json.contains("saveState")) saveState = json["saveState"];
		if (json.contains("loginOnly")) loginOnly = json["loginOnly"];
		if (json.contains("waitAfterZone")) waitAfterZone = json["waitAfterZone"];

		// Load character creation options from nested object
		if (json.contains("create_character")) {
			auto& createChar = json["create_character"];
			if (createChar.contains("enabled")) createCharacter = createChar["enabled"];
			if (createChar.contains("name")) createCharName = createChar["name"];
			if (createChar.contains("race")) createCharRace = createChar["race"];
			if (createChar.contains("profession")) createCharProfession = createChar["profession"];
			if (createChar.contains("height")) createCharHeight = createChar["height"];
			if (createChar.contains("customization")) createCharCustomization = createChar["customization"];
			if (createChar.contains("biography")) createCharBiography = createChar["biography"];
			if (createChar.contains("skipTutorial")) createCharSkipTutorial = createChar["skipTutorial"];

			if (createChar.contains("hair")) {
				auto& hair = createChar["hair"];
				if (hair.contains("template")) createCharHairTemplate = hair["template"];
				if (hair.contains("customization")) createCharHairCustomization = hair["customization"];
			}
		}

		return true;
	} catch (const std::exception& e) {
		return false;
	}
}

int ClientCoreOptions::parseArgs(int index, int argc, char** argv) {
	return 0;
}

void ClientCoreOptions::parse(int argc, char* argv[]) {
	// Save for action parsing
	this->argc = argc;
	this->argv = argv;

	namespace po = boost::program_options;

	// First pass to get --env option
	po::options_description env_desc("Environment");
	env_desc.add_options()
		("env", po::value<std::string>(), "Environment file to load");

	po::variables_map env_vm;
	po::store(po::command_line_parser(argc, argv).options(env_desc).allow_unregistered().run(), env_vm);
	po::notify(env_vm);

	// Load .env files
	if (env_vm.count("env")) {
		loadEnvFile(env_vm["env"].as<std::string>().c_str());
	} else {
		loadEnvFile(".env");
		loadEnvFile(".env-core3client");
	}

	// Load options from JSON file if specified in environment
	if (env_vm.count("options-json")) {
		String jsonFile = env_vm["options-json"].as<std::string>().c_str();
		if (!loadFromJSON(jsonFile)) {
			System::err << "Error: Failed to load options from JSON file: " << jsonFile << endl;
			exit(1);
		}
	}

	// Full options parsing
	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Show help message")
		("username,u", po::value<std::string>(), "Username for login")
		("password,p", po::value<std::string>(), "Password for login")
		("login-host", po::value<std::string>(), "Login server hostname")
		("login-port", po::value<int>(), "Login server port")
		("client-version", po::value<std::string>(), "Client version string")
		("login-timeout", po::value<int>(), "Login timeout in seconds")
		("zone-timeout", po::value<int>(), "Zone timeout in seconds")
		("log-level", po::value<std::string>(), "Log level (0-5 or fatal/error/warning/log/info/debug)")
		("character-oid", po::value<uint64>(), "Character object ID to select")
		("character-firstname", po::value<std::string>(), "Character first name to select")
		("save-state", po::value<std::string>(), "Save login state to JSON file")
		("login-only", "Only perform login, skip zone connection")
		("options-json", po::value<std::string>(), "Load options from JSON file")
		("generate-options-json", "Generate default options JSON to stdout and exit")
		("env", po::value<std::string>(), "Environment file to load")
		("wait-after-zone", po::value<int>(), "Seconds to stay connected to zone before shutdown")
		("create-character", "Automatically create character if none exist")
		("char-name", po::value<std::string>(), "Character name")
		("char-race", po::value<std::string>(), "Race template (full IFF path or short name)")
		("char-profession", po::value<std::string>(), "Starting profession")
		("char-height", po::value<float>(), "Character height (0.7-1.5)")
		("char-customization", po::value<std::string>(), "Appearance customization string")
		("char-hair-template", po::value<std::string>(), "Hair template")
		("char-hair-customization", po::value<std::string>(), "Hair customization")
		("char-biography", po::value<std::string>(), "Character biography");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// Load options from JSON file if specified on command line (before processing other options)
	if (vm.count("options-json")) {
		String jsonFile = vm["options-json"].as<std::string>().c_str();
		if (!loadFromJSON(jsonFile)) {
			System::err << "Error: Failed to load options from JSON file: " << jsonFile << endl;
			exit(1);
		}
	}

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		exit(0);
	}

	// Generate default options JSON and exit
	if (vm.count("generate-options-json")) {
		ClientCoreOptions defaults;
		std::cout << defaults.getAsJSON().dump(2) << std::endl;
		exit(0);
	}

	// Get username from command line or environment
	if (vm.count("username")) {
		username = vm["username"].as<std::string>().c_str();
	} else {
		const char* envUser = getenv("CORE3_CLIENT_USERNAME");
		if (envUser) {
			username = envUser;
		} else if (username.isEmpty()) {
			throw Exception("ERROR: Please provide --username or set CORE3_CLIENT_USERNAME environment variable");
		}
	}

	// Get password from command line or environment
	if (vm.count("password")) {
		password = resolveFileReference(vm["password"].as<std::string>().c_str());
	} else {
		const char* envPass = getenv("CORE3_CLIENT_PASSWORD");
		if (envPass) {
			password = resolveFileReference(envPass);
		} else if (password.isEmpty()) {
			throw Exception("ERROR: Please provide --password or set CORE3_CLIENT_PASSWORD environment variable");
		}
	}

	// Get login host from command line or environment
	if (vm.count("login-host")) {
		loginHost = vm["login-host"].as<std::string>().c_str();
	} else {
		const char* envHost = getenv("CORE3_CLIENT_LOGINHOST");
		if (envHost) {
			loginHost = envHost;
		}
	}

	// Get login port from command line or environment
	if (vm.count("login-port")) {
		loginPort = vm["login-port"].as<int>();
	} else {
		const char* envPort = getenv("CORE3_CLIENT_LOGINPORT");
		if (envPort) {
			loginPort = Integer::valueOf(envPort);
		}
	}

	// Get client version from command line or environment
	if (vm.count("client-version")) {
		clientVersion = vm["client-version"].as<std::string>().c_str();
	} else {
		const char* envVersion = getenv("CORE3_CLIENT_VERSION");
		if (envVersion) {
			clientVersion = envVersion;
		}
	}

	// Get login timeout from command line or environment
	if (vm.count("login-timeout")) {
		loginTimeout = vm["login-timeout"].as<int>();
	} else {
		const char* envTimeout = getenv("CORE3_CLIENT_LOGIN_TIMEOUT");
		if (envTimeout) {
			loginTimeout = Integer::valueOf(envTimeout);
		}
	}

	// Get zone timeout from command line or environment
	if (vm.count("zone-timeout")) {
		zoneTimeout = vm["zone-timeout"].as<int>();
	} else {
		const char* envTimeout = getenv("CORE3_CLIENT_ZONE_TIMEOUT");
		if (envTimeout) {
			zoneTimeout = Integer::valueOf(envTimeout);
		}
	}

	// Get log level from command line or environment
	if (vm.count("log-level")) {
		logLevel = parseLogLevel(vm["log-level"].as<std::string>().c_str());
	} else {
		const char* envLevel = getenv("CORE3_CLIENT_LOG_LEVEL");
		if (envLevel) {
			logLevel = parseLogLevel(envLevel);
		}
	}

	// Get character OID from command line or environment
	if (vm.count("character-oid")) {
		characterOid = vm["character-oid"].as<uint64>();
	} else {
		const char* envOid = getenv("CORE3_CLIENT_CHARACTER_OID");
		if (envOid) {
			characterOid = UnsignedLong::valueOf(envOid);
		}
	}

	// Get character firstname from command line or environment
	if (vm.count("character-firstname")) {
		characterFirstname = vm["character-firstname"].as<std::string>().c_str();
	} else {
		const char* envName = getenv("CORE3_CLIENT_CHARACTER_FIRSTNAME");
		if (envName) {
			characterFirstname = envName;
		}
	}

	// Get save-state filename
	if (vm.count("save-state")) {
		saveState = vm["save-state"].as<std::string>().c_str();
	}

	// Get login-only flag
	if (vm.count("login-only")) {
		loginOnly = true;
	}

	// Get wait-after-zone
	if (vm.count("wait-after-zone")) {
		waitAfterZone = vm["wait-after-zone"].as<int>();
	}

	// Character creation options
	if (vm.count("create-character")) {
		createCharacter = true;
	}

	if (vm.count("char-name")) {
		createCharName = vm["char-name"].as<std::string>().c_str();
	}

	if (vm.count("char-race")) {
		createCharRace = vm["char-race"].as<std::string>().c_str();
	}

	if (vm.count("char-profession")) {
		createCharProfession = vm["char-profession"].as<std::string>().c_str();
	}

	if (vm.count("char-height")) {
		createCharHeight = vm["char-height"].as<float>();
	}

	if (vm.count("char-customization")) {
		createCharCustomization = vm["char-customization"].as<std::string>().c_str();
	}

	if (vm.count("char-hair-template")) {
		createCharHairTemplate = vm["char-hair-template"].as<std::string>().c_str();
	}

	if (vm.count("char-hair-customization")) {
		createCharHairCustomization = vm["char-hair-customization"].as<std::string>().c_str();
	}

	if (vm.count("char-biography")) {
		createCharBiography = vm["char-biography"].as<std::string>().c_str();
	}
}

void ClientCoreOptions::updateWithProperties() {
	if (loginHost.isEmpty()) {
		loginHost = Core::getProperty("Client3.LoginHost", "127.0.0.1");
	}

	if (loginPort == 0) {
		loginPort = Core::getIntProperty("Client3.LoginPort", 44453);
	}

	if (clientVersion.isEmpty()) {
		clientVersion = Core::getProperty("Client3.ClientVersion", "20050408-18:00");
	}

	if (loginTimeout == 0) {
		loginTimeout = Core::getIntProperty("Client3.LoginTimeout", 10);
	}

	if (zoneTimeout == 0) {
		zoneTimeout = Core::getIntProperty("Client3.ZoneTimeout", 30);
	}

	if (logLevel == -1) {
		logLevel = Core::getIntProperty("Client3.LogLevel", static_cast<int>(Logger::INFO));
	}

	if (characterOid == 0) {
		characterOid = Core::getLongProperty("Client3.CharacterOid", 0);
	}

	if (characterFirstname.isEmpty()) {
		characterFirstname = Core::getProperty("Client3.CharacterFirstname", "");
	}
}

int ClientCoreOptions::parseLogLevel(const String& levelStr) {
	String level = levelStr.toLowerCase();

	if (level == "fatal") return Logger::FATAL;
	if (level == "error") return Logger::ERROR;
	if (level == "warning" || level == "warn") return Logger::WARNING;
	if (level == "log") return Logger::LOG;
	if (level == "info") return Logger::INFO;
	if (level == "debug") return Logger::DEBUG;

	// Try to parse as number
	try {
		return Integer::valueOf(levelStr);
	} catch (...) {
		return Logger::INFO; // Default to INFO on parse failure
	}
}

void ClientCoreOptions::saveToProperties() {
	if (!loginHost.isEmpty()) {
		Core::setProperty("Client3.LoginHost", loginHost);
	}

	if (loginPort != 0) {
		Core::setProperty("Client3.LoginPort", String::valueOf(loginPort));
	}

	if (!clientVersion.isEmpty()) {
		Core::setProperty("Client3.ClientVersion", clientVersion);
	}

	if (loginTimeout != 0) {
		Core::setProperty("Client3.LoginTimeout", String::valueOf(loginTimeout));
	}

	if (zoneTimeout != 0) {
		Core::setProperty("Client3.ZoneTimeout", String::valueOf(zoneTimeout));
	}

	if (logLevel != -1) {
		Core::setIntProperty("Client3.LogLevel", logLevel);
	}

	if (characterOid != 0) {
		Core::setLongProperty("Client3.CharacterOid", characterOid);
	}

	if (!characterFirstname.isEmpty()) {
		Core::setProperty("Client3.CharacterFirstname", characterFirstname);
	}
}

String ClientCoreOptions::toString() const {
	return getAsJSON().dump().c_str();
}

JSONSerializationType ClientCoreOptions::getAsJSON() const {
	JSONSerializationType jsonData;
	jsonData["username"] = username.toCharArray();
	jsonData["password"] = "***";
	jsonData["loginHost"] = loginHost.toCharArray();
	jsonData["loginPort"] = loginPort;
	jsonData["clientVersion"] = clientVersion.toCharArray();
	jsonData["loginTimeout"] = loginTimeout;
	jsonData["zoneTimeout"] = zoneTimeout;
	jsonData["logLevel"] = logLevel;
	jsonData["characterOid"] = characterOid;
	jsonData["characterFirstname"] = characterFirstname.toCharArray();
	jsonData["saveState"] = saveState.toCharArray();
	jsonData["loginOnly"] = loginOnly;
	jsonData["waitAfterZone"] = waitAfterZone;

	// Nested create_character object
	JSONSerializationType createChar;
	createChar["enabled"] = createCharacter;
	createChar["name"] = createCharName.toCharArray();
	createChar["race"] = createCharRace.toCharArray();
	createChar["profession"] = createCharProfession.toCharArray();
	createChar["height"] = createCharHeight;
	createChar["customization"] = createCharCustomization.toCharArray();

	JSONSerializationType hair;
	hair["template"] = createCharHairTemplate.toCharArray();
	hair["customization"] = createCharHairCustomization.toCharArray();
	createChar["hair"] = hair;

	createChar["biography"] = createCharBiography.toCharArray();
	createChar["skipTutorial"] = createCharSkipTutorial;
	jsonData["create_character"] = createChar;

	return jsonData;
}

String ClientCoreOptions::toStringData() const {
	return toString();
}

int main(int argc, char* argv[]) {
	try {
		ClientCoreOptions opts;
		opts.parse(argc, argv);

		StackTrace::setBinaryName("core3client");

		// Configure engine3
		Core::setProperty("TaskManager.defaultSchedulerThreads", "2");
		Core::setProperty("TaskManager.defaultIOSchedulers", "2");
		Core::setProperty("TaskManager.defaultWorkerQueues", "1");
		Core::setProperty("TaskManager.defaultWorkerThreadsPerQueue", "2");

		ClientCore core(opts);
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
