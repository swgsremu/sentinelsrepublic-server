-- Player Action Logger for SWGEmu
-- Logs player actions to files and database for CSR monitoring

PlayerActionLogger = {}

-- Log types
PlayerActionLogger.LOG_MOVEMENT = "movement"
PlayerActionLogger.LOG_COMBAT = "combat"
PlayerActionLogger.LOG_TRADE = "trade"
PlayerActionLogger.LOG_LOOT = "loot"
PlayerActionLogger.LOG_QUEST = "quest"
PlayerActionLogger.LOG_SOCIAL = "social"
PlayerActionLogger.LOG_CONNECTION = "connection"
PlayerActionLogger.LOG_ADMIN = "admin"

-- Initialize logger
function PlayerActionLogger:init()
    print("[CSR] Player action logger initialized")
end

-- Main logging function
function PlayerActionLogger:logAction(player, logType, action, details)
    if not player or not SceneObject(player):isPlayerCreature() then
        return
    end
    
    local creature = LuaCreatureObject(player)
    local timestamp = os.date("[%Y-%m-%d %H:%M:%S]")
    local playerName = creature:getFirstName()
    local accountId = creature:getAccountID()
    local oid = creature:getObjectID()
    
    -- Get location
    local x = creature:getPositionX()
    local y = creature:getPositionY()
    local z = creature:getPositionZ()
    local sceneObject = SceneObject(player)
    local zone = sceneObject:getZone()
    local planet = zone and zone:getZoneName() or "unknown"
    
    -- Log to file
    self:logToFile(timestamp, logType, playerName, action, details, planet, x, y, z)
    
    -- Log to database
    self:logToDatabase(oid, accountId, playerName, logType, action, details, x, y, z, planet)
end

-- Log to file
function PlayerActionLogger:logToFile(timestamp, logType, playerName, action, details, planet, x, y, z)
    local logFile = string.format("logs/%s.log", logType)
    local logEntry = string.format("%s %s: %s %s at %s (%.1f, %.1f, %.1f)",
        timestamp, string.upper(logType), playerName, action, planet, x, y, z)
    
    if details and details ~= "" then
        logEntry = logEntry .. " - " .. details
    end
    
    -- Write to file
    local file = io.open(logFile, "a")
    if file then
        file:write(logEntry .. "\n")
        file:close()
    end
end

-- Log to database
function PlayerActionLogger:logToDatabase(oid, accountId, playerName, logType, action, details, x, y, z, planet)
    local detailsJson = "{}"
    if details then
        -- Simple JSON encoding for details
        detailsJson = string.format('{"details": "%s"}', details:gsub('"', '\\"'))
    end
    
    local query = string.format([[
        INSERT INTO player_logs 
        (character_oid, account_id, character_name, log_type, action, details, 
         location_x, location_y, location_z, planet, timestamp) 
        VALUES (%d, %d, '%s', '%s', '%s', '%s', %.2f, %.2f, %.2f, '%s', NOW())
    ]], oid, accountId, playerName:gsub("'", "''"), logType, action, detailsJson:gsub("'", "''"), x, y, z, planet)
    
    local success, result = pcall(function()
        return getDatabase():executeUpdate(query)
    end)
    
    if not success then
        print("[CSR] Failed to log action to database: " .. tostring(result))
    end
end

-- Specific logging functions

-- Log player login
function PlayerActionLogger:logLogin(player)
    self:logAction(player, self.LOG_CONNECTION, "logged in", "")
end

-- Log player logout
function PlayerActionLogger:logLogout(player)
    self:logAction(player, self.LOG_CONNECTION, "logged out", "")
end

-- Log zone change
function PlayerActionLogger:logZoneChange(player, oldZone, newZone)
    local details = string.format("from %s to %s", 
        oldZone and oldZone:getZoneName() or "unknown",
        newZone and newZone:getZoneName() or "unknown")
    self:logAction(player, self.LOG_MOVEMENT, "changed zone", details)
end

-- Log combat action
function PlayerActionLogger:logCombat(attacker, defender, damage, weapon)
    if attacker and SceneObject(attacker):isPlayerCreature() then
        local defenderName = "unknown"
        if defender then
            local defenderObj = SceneObject(defender)
            if defenderObj:isPlayerCreature() then
                defenderName = LuaCreatureObject(defender):getFirstName()
            else
                defenderName = defenderObj:getDisplayedName()
            end
        end
        
        local details = string.format("dealt %d damage to %s with %s",
            damage, defenderName, weapon or "unknown weapon")
        self:logAction(attacker, self.LOG_COMBAT, "attacked", details)
    end
end

-- Log trade
function PlayerActionLogger:logTrade(player1, player2, itemsTraded, creditsTraded)
    local creature2 = LuaCreatureObject(player2)
    local details = string.format("with %s - %d items, %d credits",
        creature2:getFirstName(), itemsTraded, creditsTraded)
    self:logAction(player1, self.LOG_TRADE, "traded", details)
    
    -- Log for both players
    local creature1 = LuaCreatureObject(player1)
    details = string.format("with %s - %d items, %d credits",
        creature1:getFirstName(), itemsTraded, creditsTraded)
    self:logAction(player2, self.LOG_TRADE, "traded", details)
end

-- Log loot
function PlayerActionLogger:logLoot(player, item, container)
    local itemName = "unknown item"
    if item then
        itemName = SceneObject(item):getDisplayedName()
    end
    
    local containerName = "container"
    if container then
        containerName = SceneObject(container):getDisplayedName()
    end
    
    local details = string.format("%s from %s", itemName, containerName)
    self:logAction(player, self.LOG_LOOT, "looted", details)
end

-- Log quest completion
function PlayerActionLogger:logQuestComplete(player, questName)
    self:logAction(player, self.LOG_QUEST, "completed quest", questName)
end

-- Log social action (friend, ignore, etc)
function PlayerActionLogger:logSocial(player, action, targetName)
    local details = targetName or ""
    self:logAction(player, self.LOG_SOCIAL, action, details)
end

-- Log admin command usage
function PlayerActionLogger:logAdminCommand(admin, command, target)
    local details = command
    if target then
        details = details .. " on " .. target
    end
    self:logAction(admin, self.LOG_ADMIN, "used admin command", details)
end

-- Initialize the logger
PlayerActionLogger:init()

-- Make globally accessible
_G.PlayerActionLogger = PlayerActionLogger

return PlayerActionLogger