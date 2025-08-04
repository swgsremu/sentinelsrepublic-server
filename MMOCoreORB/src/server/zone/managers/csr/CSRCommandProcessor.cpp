/*
 * CSRCommandProcessor.cpp
 */

#include "CSRCommandProcessor.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/packets/player/LogoutMessage.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/db/ServerDatabase.h"
#include "server/chat/ChatManager.h"
#include "engine/db/Database.h"

namespace server {
namespace zone {
namespace managers {
namespace csr {

CSRCommandProcessor* CSRCommandProcessor::instance = nullptr;
Mutex CSRCommandProcessor::instanceMutex;

CSRCommandProcessor::CSRCommandProcessor(ZoneServer* server) : 
    Task(), 
    Logger("CSRCommandProcessor"),
    zoneServer(server), 
    isRunning(false),
    pollInterval(2000) { // 2 seconds
}

CSRCommandProcessor::~CSRCommandProcessor() {
    stop();
}

void CSRCommandProcessor::start() {
    if (isRunning) {
        return;
    }
    
    isRunning = true;
    info("CSR Command Processor starting", true);
    
    // Schedule first poll
    schedule(pollInterval);
}

void CSRCommandProcessor::stop() {
    isRunning = false;
    cancel();
    info("CSR Command Processor stopped", true);
}

void CSRCommandProcessor::run() {
    if (!isRunning) {
        return;
    }
    
    processPendingCommands();
    
    // Schedule next poll
    if (isRunning) {
        schedule(pollInterval);
    }
}

void CSRCommandProcessor::processPendingCommands() {
    try {
        // Query for pending commands
        String query = "SELECT id, command, parameters, issued_by FROM csr_bot_commands "
                      "WHERE status = 'pending' ORDER BY issued_at ASC LIMIT 10";
        
        Reference<ResultSet*> res = ServerDatabase::instance()->executeQuery(query);
        
        while (res != nullptr && res->next()) {
            int commandId = res->getUnsignedInt(0);
            String command = res->getString(1);
            String parameters = res->getString(2);
            String issuedBy = res->getString(3);
            
            info("Processing CSR command: " + command + " (ID: " + String::valueOf(commandId) + ")", true);
            
            // Execute the command
            executeCommand(commandId, command, parameters, issuedBy);
        }
        
    } catch (DatabaseException& e) {
        error("Database error: " + e.getMessage());
    } catch (Exception& e) {
        error("Error processing commands: " + e.getMessage());
    }
}

bool CSRCommandProcessor::executeCommand(int commandId, const String& command, 
                                        const String& parameters, const String& issuedBy) {
    bool success = false;
    String result = "";
    
    try {
        if (command == "/systemMessage") {
            success = executeSystemMessage(parameters, result);
        } else if (command == "/freezePlayer") {
            success = executeFreezePlayer(parameters, result);
        } else if (command == "/unfreezePlayer") {
            success = executeUnfreezePlayer(parameters, result);
        } else if (command == "/kick") {
            success = executeKickPlayer(parameters, result);
        } else if (command == "/teleportTarget") {
            success = executeTeleportPlayer(parameters, result);
        } else if (command == "/broadcast") {
            success = executeBroadcast(parameters, result);
        } else if (command == "/grantCredits") {
            success = executeGrantCredits(parameters, result);
        } else {
            result = "Unknown command: " + command;
        }
    } catch (Exception& e) {
        success = false;
        result = "Error: " + e.getMessage();
    }
    
    // Update command status
    updateCommandStatus(commandId, success ? "executed" : "failed", result);
    
    return success;
}

Reference<CreatureObject*> CSRCommandProcessor::findPlayerByName(const String& name) {
    auto playerManager = zoneServer->getPlayerManager();
    if (playerManager == nullptr) {
        return nullptr;
    }
    
    return playerManager->getPlayer(name);
}

bool CSRCommandProcessor::executeSystemMessage(const String& parameters, String& result) {
    // Parse parameters: characterName message
    int spaceIndex = parameters.indexOf(" ");
    if (spaceIndex == -1) {
        result = "Invalid parameters format";
        return false;
    }
    
    String characterName = parameters.subString(0, spaceIndex);
    String message = parameters.subString(spaceIndex + 1);
    
    // Find player
    auto player = findPlayerByName(characterName);
    if (player == nullptr) {
        result = "Player not found or offline: " + characterName;
        return false;
    }
    
    // Send system message
    player->sendSystemMessage("[CSR Message] " + message);
    
    result = "Message sent to " + characterName;
    return true;
}

bool CSRCommandProcessor::executeFreezePlayer(const String& parameters, String& result) {
    // Parse parameters: characterName reason
    int spaceIndex = parameters.indexOf(" ");
    String characterName;
    String reason = "No reason provided";
    
    if (spaceIndex != -1) {
        characterName = parameters.subString(0, spaceIndex);
        reason = parameters.subString(spaceIndex + 1);
    } else {
        characterName = parameters;
    }
    
    // Find player
    auto player = findPlayerByName(characterName);
    if (player == nullptr) {
        result = "Player not found or offline: " + characterName;
        return false;
    }
    
    Locker locker(player);
    
    // Freeze player (set root state)
    player->setRootedState(true);
    player->sendSystemMessage("You have been frozen by a CSR. Reason: " + reason);
    
    result = "Player frozen: " + characterName;
    return true;
}

bool CSRCommandProcessor::executeUnfreezePlayer(const String& parameters, String& result) {
    String characterName = parameters.trim();
    
    // Find player
    auto player = findPlayerByName(characterName);
    if (player == nullptr) {
        result = "Player not found or offline: " + characterName;
        return false;
    }
    
    Locker locker(player);
    
    // Unfreeze player
    player->setRootedState(false);
    player->sendSystemMessage("You have been unfrozen by a CSR.");
    
    result = "Player unfrozen: " + characterName;
    return true;
}

bool CSRCommandProcessor::executeKickPlayer(const String& parameters, String& result) {
    // Parse parameters: characterName reason
    int spaceIndex = parameters.indexOf(" ");
    String characterName;
    String reason = "No reason provided";
    
    if (spaceIndex != -1) {
        characterName = parameters.subString(0, spaceIndex);
        reason = parameters.subString(spaceIndex + 1);
    } else {
        characterName = parameters;
    }
    
    // Find player
    auto player = findPlayerByName(characterName);
    if (player == nullptr) {
        result = "Player not found or offline: " + characterName;
        return false;
    }
    
    Locker locker(player);
    
    // Send kick message
    player->sendSystemMessage("You are being disconnected by a CSR. Reason: " + reason);
    
    // Disconnect player
    auto ghost = player->getPlayerObject();
    if (ghost != nullptr) {
        player->sendMessage(new LogoutMessage());
        ghost->disconnect(true, false);
    }
    
    result = "Player kicked: " + characterName;
    return true;
}

bool CSRCommandProcessor::executeTeleportPlayer(const String& parameters, String& result) {
    // Parse parameters: characterName planet x y z
    StringTokenizer tokenizer(parameters);
    tokenizer.setDelimeter(" ");
    
    if (!tokenizer.hasMoreTokens()) {
        result = "Invalid parameters";
        return false;
    }
    
    String characterName = tokenizer.getStringToken();
    
    if (!tokenizer.hasMoreTokens()) {
        result = "Missing planet";
        return false;
    }
    String planet = tokenizer.getStringToken();
    
    float x = 0, y = 0, z = 0;
    try {
        if (tokenizer.hasMoreTokens()) x = tokenizer.getFloatToken();
        if (tokenizer.hasMoreTokens()) y = tokenizer.getFloatToken();
        if (tokenizer.hasMoreTokens()) z = tokenizer.getFloatToken();
    } catch (...) {
        result = "Invalid coordinates";
        return false;
    }
    
    // Find player
    auto player = findPlayerByName(characterName);
    if (player == nullptr) {
        result = "Player not found or offline: " + characterName;
        return false;
    }
    
    Locker locker(player);
    
    // Teleport player
    player->switchZone(planet, x, z, y, 0);
    player->sendSystemMessage("You have been teleported by a CSR.");
    
    result = "Player teleported: " + characterName;
    return true;
}

bool CSRCommandProcessor::executeBroadcast(const String& parameters, String& result) {
    String message = "[CSR Broadcast] " + parameters;
    
    // Get chat manager and broadcast
    auto chatManager = zoneServer->getChatManager();
    if (chatManager != nullptr) {
        chatManager->broadcastGalaxy(nullptr, message);
        result = "Broadcast sent";
        return true;
    }
    
    result = "Failed to get chat manager";
    return false;
}

bool CSRCommandProcessor::executeGrantCredits(const String& parameters, String& result) {
    // Parse parameters: characterName amount
    int spaceIndex = parameters.indexOf(" ");
    if (spaceIndex == -1) {
        result = "Invalid parameters format";
        return false;
    }
    
    String characterName = parameters.subString(0, spaceIndex);
    int amount = 0;
    
    try {
        amount = Integer::valueOf(parameters.subString(spaceIndex + 1));
    } catch (...) {
        result = "Invalid credit amount";
        return false;
    }
    
    if (amount <= 0) {
        result = "Credit amount must be positive";
        return false;
    }
    
    // Find player
    auto player = findPlayerByName(characterName);
    if (player == nullptr) {
        result = "Player not found or offline: " + characterName;
        return false;
    }
    
    Locker locker(player);
    
    // Grant credits
    player->addCashCredits(amount, true);
    player->sendSystemMessage("You have been granted " + String::valueOf(amount) + " credits by a CSR.");
    
    result = "Granted " + String::valueOf(amount) + " credits to " + characterName;
    return true;
}

void CSRCommandProcessor::updateCommandStatus(int commandId, const String& status, const String& result) {
    try {
        // Create copies for escaping
        String escapedStatus = status;
        String escapedResult = result;
        
        // Escape strings to prevent SQL injection
        Database::escapeString(escapedStatus);
        Database::escapeString(escapedResult);
        
        StringBuffer query;
        query << "UPDATE csr_bot_commands SET status = '" << escapedStatus 
              << "', executed_at = NOW(), result = '" << escapedResult
              << "' WHERE id = " << commandId;
        
        ServerDatabase::instance()->executeStatement(query.toString());
        
    } catch (DatabaseException& e) {
        error("Failed to update command status: " + e.getMessage());
    }
}

} // namespace csr
} // namespace managers
} // namespace zone
} // namespace server