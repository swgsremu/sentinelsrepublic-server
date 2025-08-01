#include "AdminCommandLogger.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/db/ServerDatabase.h"
#include "engine/db/Database.h"

AdminCommandLogger::AdminCommandLogger() : Logger("AdminCommandLogger") {
    info("Admin Command Logger initialized", true);
}

AdminCommandLogger::~AdminCommandLogger() {
}

void AdminCommandLogger::logCommand(CreatureObject* admin, const String& command, const String& arguments, bool success, const String& errorMessage) {
    if (admin == nullptr) {
        return;
    }
    
    // Get player object for account info
    auto ghost = admin->getPlayerObject();
    if (ghost == nullptr) {
        return;
    }
    
    int adminLevel = ghost->getAdminLevel();
    
    // Only log commands from admins (level > 0)
    if (adminLevel <= 0) {
        return;
    }
    
    // Log to console
    info("Admin Command: [" + admin->getFirstName() + " (Level " + String::valueOf(adminLevel) + ")] " + 
         command + " " + arguments + " - " + (success ? "SUCCESS" : "FAILED"), true);
    
    // Write to database
    writeToDatabase(admin, command, arguments, success, errorMessage);
}

void AdminCommandLogger::writeToDatabase(CreatureObject* admin, const String& command, const String& arguments, bool success, const String& errorMessage) {
    try {
        auto ghost = admin->getPlayerObject();
        if (ghost == nullptr) {
            return;
        }
        
        String adminName = admin->getFirstName();
        int accountID = ghost->getAccountID();
        int64 characterOID = admin->getObjectID();
        
        // Parse target from arguments if possible
        String targetCharacterName = "";
        int64 targetCharacterOID = 0;
        int targetAccountID = 0;
        
        // For commands that target a player, try to extract the first argument as target
        StringTokenizer tokenizer(arguments);
        if (tokenizer.hasMoreTokens()) {
            String firstArg = tokenizer.getStringToken();
            
            // Common admin commands that target players
            if (command.contains("teleport") || command.contains("kick") || 
                command.contains("ban") || command.contains("freeze") ||
                command.contains("setspeed") || command.contains("heal") ||
                command.contains("kill") || command.contains("revive")) {
                targetCharacterName = firstArg;
            }
        }
        
        // Escape strings for SQL
        String commandCopy = command;
        String argsCopy = arguments;
        String adminNameCopy = adminName;
        String targetNameCopy = targetCharacterName;
        String errorCopy = errorMessage;
        
        Database::escapeString(commandCopy);
        Database::escapeString(argsCopy);
        Database::escapeString(adminNameCopy);
        Database::escapeString(targetNameCopy);
        Database::escapeString(errorCopy);
        
        StringBuffer query;
        query << "INSERT INTO admin_command_logs "
              << "(command, parameters, target_character_name, target_character_oid, "
              << "target_account_id, issued_by, issued_by_account_id, result, error_message) "
              << "VALUES ('" << commandCopy << "', '" << argsCopy << "', ";
        
        if (targetNameCopy.isEmpty()) {
            query << "NULL, ";
        } else {
            query << "'" << targetNameCopy << "', ";
        }
        
        query << "NULL, NULL, "; // target OID and account ID - could be looked up if needed
        
        query << "'" << adminNameCopy << "', " << accountID << ", '"
              << (success ? "executed" : "failed") << "', '" << errorCopy << "')";
        
        ServerDatabase::instance()->executeStatement(query.toString());
        
    } catch (Exception& e) {
        error("Failed to log admin command: " + e.getMessage());
    }
}