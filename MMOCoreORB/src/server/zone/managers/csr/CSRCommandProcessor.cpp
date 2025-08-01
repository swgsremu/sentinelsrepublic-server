#include "CSRCommandProcessor.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/db/ServerDatabase.h"
#include "engine/db/Database.h"
#include "server/zone/packets/player/PlayMusicMessage.h"
#include "server/zone/Zone.h"
#include "server/chat/ChatManager.h"

CSRCommandProcessor::CSRCommandProcessor(ZoneServer* server) : Task(), Logger("CSRCommandProcessor") {
    zoneServer = server;
    running = true;
    taskName = "CSRCommandProcessor";
    
    info("CSR Command Processor initialized", true);
}

CSRCommandProcessor::~CSRCommandProcessor() {
    stop();
}

void CSRCommandProcessor::run() {
    if (!running) {
        return;
    }
    
    processCommands();
    
    // Reschedule to run again in 2 seconds
    if (running) {
        schedule(2000);
    }
}

void CSRCommandProcessor::stop() {
    running = false;
}

void CSRCommandProcessor::processCommands() {
    try {
        Reference<ResultSet*> result = ServerDatabase::instance()->executeQuery(
            "SELECT id, command, parameters, issued_by FROM csr_bot_commands "
            "WHERE status = 'pending' ORDER BY issued_at ASC LIMIT 10"
        );
        
        while (result->next()) {
            int commandId = result->getInt(0);
            String command = result->getString(1);
            String parameters = result->getString(2);
            String issuedBy = result->getString(3);
            
            info("Processing CSR command: " + command + " with parameters: " + parameters, true);
            
            executeCommand(command, parameters, issuedBy, commandId);
        }
    } catch (Exception& e) {
        error("Error processing CSR commands: " + e.getMessage());
    }
}

void CSRCommandProcessor::executeCommand(const String& command, const String& parameters, const String& issuedBy, int commandId) {
    String targetCharacterName = "";
    int64 targetCharacterOID = 0;
    int targetAccountID = 0;
    String result = "";
    String errorMsg = "";
    
    try {
        // Extract target character name from parameters (first token for most commands)
        StringTokenizer tokenizer(parameters);
        if (tokenizer.hasMoreTokens() && command != "/broadcast") {
            targetCharacterName = tokenizer.getStringToken();
        }
        
        if (command == "/freezePlayer") {
            StringTokenizer tokenizer(parameters);
            String playerName = tokenizer.getStringToken();
            String reason = tokenizer.getRemainingString();
            
            // Get player info for logging
            auto playerManager = zoneServer->getPlayerManager();
            Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
            if (player != nullptr) {
                targetCharacterOID = player->getObjectID();
                auto ghost = player->getPlayerObject();
                if (ghost != nullptr) {
                    targetAccountID = ghost->getAccountID();
                }
            }
            
            freezePlayer(playerName, reason);
            updateCommandStatus(commandId, "executed", "Player frozen");
            
        } else if (command == "/unfreezePlayer") {
            unfreezePlayer(parameters.trim());
            updateCommandStatus(commandId, "executed", "Player unfrozen");
            
        } else if (command == "/kickPlayer") {
            StringTokenizer tokenizer(parameters);
            String playerName = tokenizer.getStringToken();
            String reason = tokenizer.getRemainingString();
            
            kickPlayer(playerName, reason);
            updateCommandStatus(commandId, "executed", "Player kicked");
            
        } else if (command == "/teleport") {
            StringTokenizer tokenizer(parameters);
            String playerName = tokenizer.getStringToken();
            float x = tokenizer.getFloatToken();
            float y = tokenizer.getFloatToken();
            float z = tokenizer.getFloatToken();
            String planet = tokenizer.getStringToken();
            
            teleportPlayer(playerName, x, y, z, planet);
            updateCommandStatus(commandId, "executed", "Player teleported");
            
        } else if (command == "/systemMessage") {
            StringTokenizer tokenizer(parameters);
            String playerName = tokenizer.getStringToken();
            String message = tokenizer.getRemainingString();
            
            sendSystemMessage(playerName, message);
            updateCommandStatus(commandId, "executed", "Message sent");
            
        } else if (command == "/broadcast") {
            sendBroadcast(parameters.trim());
            updateCommandStatus(commandId, "executed", "Broadcast sent");
            
        } else if (command == "/grantCredits") {
            StringTokenizer tokenizer(parameters);
            String playerName = tokenizer.getStringToken();
            int amount = tokenizer.getIntToken();
            
            grantCredits(playerName, amount);
            updateCommandStatus(commandId, "executed", "Credits granted");
            
        } else {
            errorMsg = "Unknown command: " + command;
            updateCommandStatus(commandId, "failed", errorMsg);
        }
        
        // Log successful command
        if (errorMsg.isEmpty()) {
            logAdminCommand(command, parameters, targetCharacterName, targetCharacterOID, targetAccountID, issuedBy, "executed", "");
        }
        
    } catch (Exception& e) {
        errorMsg = "Error: " + e.getMessage();
        updateCommandStatus(commandId, "failed", errorMsg);
        
        // Log failed command
        logAdminCommand(command, parameters, targetCharacterName, targetCharacterOID, targetAccountID, issuedBy, "failed", errorMsg);
    }
}

void CSRCommandProcessor::updateCommandStatus(int commandId, const String& status, const String& result) {
    try {
        // Create copies for escapeString
        String statusCopy = status;
        String resultCopy = result;
        
        Database::escapeString(statusCopy);
        Database::escapeString(resultCopy);
        
        StringBuffer query;
        query << "UPDATE csr_bot_commands SET status = '" << statusCopy
              << "', result = '" << resultCopy
              << "', executed_at = NOW() WHERE id = " << commandId;
              
        ServerDatabase::instance()->executeStatement(query.toString());
    } catch (Exception& e) {
        error("Failed to update command status: " + e.getMessage());
    }
}

void CSRCommandProcessor::freezePlayer(const String& playerName, const String& reason) {
    auto playerManager = zoneServer->getPlayerManager();
    Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
    
    if (player == nullptr) {
        throw Exception("Player not found: " + playerName);
    }
    
    Locker locker(player);
    
    // Set speed to 0 to freeze the player
    player->setSpeedMultiplierBase(0.0f);
    player->setAccelerationMultiplierBase(0.0f);
    player->setTurnScale(0.0f);
    
    // Send a message to the player
    player->sendSystemMessage("You have been frozen by a CSR. Reason: " + reason);
    
    // Play freeze sound
    PlayMusicMessage* pmm = new PlayMusicMessage("sound/ui_freeze.snd");
    player->sendMessage(pmm);
    
    info("Froze player: " + playerName + " Reason: " + reason, true);
}

void CSRCommandProcessor::unfreezePlayer(const String& playerName) {
    auto playerManager = zoneServer->getPlayerManager();
    Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
    
    if (player == nullptr) {
        throw Exception("Player not found: " + playerName);
    }
    
    Locker locker(player);
    
    // Restore normal movement
    player->setSpeedMultiplierBase(1.0f);
    player->setAccelerationMultiplierBase(1.0f);
    player->setTurnScale(1.0f);
    
    // Send a message to the player
    player->sendSystemMessage("You have been unfrozen by a CSR.");
    
    info("Unfroze player: " + playerName, true);
}

void CSRCommandProcessor::kickPlayer(const String& playerName, const String& reason) {
    auto playerManager = zoneServer->getPlayerManager();
    Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
    
    if (player == nullptr) {
        throw Exception("Player not found: " + playerName);
    }
    
    Locker locker(player);
    
    // Send kick message
    player->sendSystemMessage("You have been kicked from the server by a CSR. Reason: " + reason);
    
    // Disconnect the player
    auto playerObject = player->getPlayerObject();
    if (playerObject != nullptr) {
        playerObject->disconnect(true, true);
    }
    
    info("Kicked player: " + playerName + " Reason: " + reason, true);
}

void CSRCommandProcessor::teleportPlayer(const String& playerName, float x, float y, float z, const String& planet) {
    auto playerManager = zoneServer->getPlayerManager();
    Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
    
    if (player == nullptr) {
        throw Exception("Player not found: " + playerName);
    }
    
    Locker locker(player);
    
    // Find the zone for the planet
    Zone* zone = zoneServer->getZone(planet);
    if (zone == nullptr) {
        throw Exception("Invalid planet: " + planet);
    }
    
    // Teleport the player
    player->switchZone(zone->getZoneName(), x, z, y, 0);
    
    // Send a message to the player
    player->sendSystemMessage("You have been teleported by a CSR.");
    
    info("Teleported player: " + playerName + " to " + planet + " (" + String::valueOf(x) + ", " + String::valueOf(y) + ", " + String::valueOf(z) + ")", true);
}

void CSRCommandProcessor::sendSystemMessage(const String& playerName, const String& message) {
    auto playerManager = zoneServer->getPlayerManager();
    Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
    
    if (player == nullptr) {
        throw Exception("Player not found: " + playerName);
    }
    
    Locker locker(player);
    
    // Send system message
    player->sendSystemMessage(message);
    
    info("Sent system message to " + playerName + ": " + message, true);
}

void CSRCommandProcessor::sendBroadcast(const String& message) {
    auto chatManager = zoneServer->getChatManager();
    
    if (chatManager == nullptr) {
        throw Exception("Chat manager not available");
    }
    
    // Send server-wide broadcast
    chatManager->broadcastGalaxy(nullptr, "[CSR Broadcast] " + message);
    
    info("Sent broadcast: " + message, true);
}

void CSRCommandProcessor::grantCredits(const String& playerName, int amount) {
    auto playerManager = zoneServer->getPlayerManager();
    Reference<CreatureObject*> player = playerManager->getPlayer(playerName);
    
    if (player == nullptr) {
        throw Exception("Player not found: " + playerName);
    }
    
    Locker locker(player);
    
    // Add credits
    if (amount > 0) {
        player->addCashCredits(amount, true);
        player->sendSystemMessage("You have been granted " + String::valueOf(amount) + " credits by a CSR.");
    } else if (amount < 0) {
        player->subtractCashCredits(-amount);
        player->sendSystemMessage("A CSR has removed " + String::valueOf(-amount) + " credits from your account.");
    }
    
    info("Granted " + String::valueOf(amount) + " credits to " + playerName, true);
}

void CSRCommandProcessor::logAdminCommand(const String& command, const String& parameters, const String& targetCharName, 
                                         int64 targetCharOID, int targetAccID, const String& issuedBy, 
                                         const String& result, const String& errorMsg) {
    try {
        // Get account ID for the issuer
        int issuedByAccountID = 0;
        String query = "SELECT account_id FROM accounts WHERE username = '" + issuedBy + "' LIMIT 1";
        Reference<ResultSet*> res = ServerDatabase::instance()->executeQuery(query);
        if (res->next()) {
            issuedByAccountID = res->getInt(0);
        }
        
        // Escape strings for SQL
        String commandCopy = command;
        String paramsCopy = parameters;
        String targetNameCopy = targetCharName;
        String issuedByCopy = issuedBy;
        String resultCopy = result;
        String errorCopy = errorMsg;
        
        Database::escapeString(commandCopy);
        Database::escapeString(paramsCopy);
        Database::escapeString(targetNameCopy);
        Database::escapeString(issuedByCopy);
        Database::escapeString(resultCopy);
        Database::escapeString(errorCopy);
        
        StringBuffer insertQuery;
        insertQuery << "INSERT INTO admin_command_logs "
                   << "(command, parameters, target_character_name, target_character_oid, "
                   << "target_account_id, issued_by, issued_by_account_id, result, error_message) "
                   << "VALUES ('" << commandCopy << "', '" << paramsCopy << "', ";
        
        if (targetNameCopy.isEmpty()) {
            insertQuery << "NULL, ";
        } else {
            insertQuery << "'" << targetNameCopy << "', ";
        }
        
        if (targetCharOID == 0) {
            insertQuery << "NULL, ";
        } else {
            insertQuery << targetCharOID << ", ";
        }
        
        if (targetAccID == 0) {
            insertQuery << "NULL, ";
        } else {
            insertQuery << targetAccID << ", ";
        }
        
        insertQuery << "'" << issuedByCopy << "', " << issuedByAccountID << ", '"
                   << resultCopy << "', '" << errorCopy << "')";
        
        ServerDatabase::instance()->executeStatement(insertQuery.toString());
        
    } catch (Exception& e) {
        error("Failed to log admin command: " + e.getMessage());
    }
}