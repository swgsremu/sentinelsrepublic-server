#ifdef WITH_DPP
#include "DiscordBot.h"
#include "server/zone/packets/chat/ChatRoomMessage.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/objects/guild/GuildObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/login/account/Account.h"
#include "templates/faction/Factions.h"
#include "conf/ConfigManager.h"
#include "engine/log/Logger.h"

DiscordBot::DiscordBot() : Logger("DiscordBot") {
	setInfoLogLevel();
}

DiscordBot::~DiscordBot() {
	if (bot != nullptr) {
		delete bot;
		bot = nullptr;
	}
}

void DiscordBot::InitializeBot(String botName, String botToken) {
	info("InitializeBot called");
	
	if (botName.isEmpty()) {
		error("InitializeBot: botName must not be empty!");
		fatal("botName must not be empty!");
	}

	if (botToken.isEmpty()) {
		error("InitializeBot: botToken must not be empty!");
		fatal("botToken must not be empty!");
	}

	StringBuffer initMsg;
	initMsg << "Initializing Discord bot with name: " << botName;
	info(initMsg);

	name = botName;
	bot = new dpp::cluster(std::string(botToken), dpp::i_default_intents | dpp::i_message_content);

	pvpLogChannelId = ConfigManager::instance()->getString("Core3.Discord.PvPLogChannelId", "");
	
	StringBuffer pvpMsg;
	pvpMsg << "PvP log channel ID: '" << pvpLogChannelId << "'";
	info(pvpMsg);

	// Use standard Core3 logging for consistency
	bot->on_log([this](const dpp::log_t & log) { HandleDppLogEvent(log); });

	info("Discord bot initialized successfully");
}

void DiscordBot::Start() {
	info("Discord bot Start() called");
	
	if (bot == nullptr) {
		error("Cannot start Discord bot - not initialized");
		warning("cannot start - not initialized");
		return;
	}

	info("Starting Discord bot...");
	bot->start(dpp::st_return);
	info("Discord bot start command issued");
}

void DiscordBot::Shutdown() {
	if (bot == nullptr) {
		// nothing to shutdown
		return;
	}

	bot->shutdown();
	info("shutdown");
}

void DiscordBot::AddMonitoredChatRoom(ChatRoom* room) {
	if (bot == nullptr) {
		warning("AddMonitoredChatRoom: bot is null - not configured on startup");
		return;
	}

	auto monitoredChatRoom = room;
	auto discordChannelId = room->getDiscordChannelId();

	StringBuffer addRoomLogMsg;
	addRoomLogMsg << "Setting up synchronized chat for " << monitoredChatRoom->getFullPath();
	info(addRoomLogMsg);

	StringBuffer channelLogMsg;
	channelLogMsg << "Discord channel ID for room '" << monitoredChatRoom->getFullPath() << "': '" << discordChannelId << "'";
	info(channelLogMsg);

	if (discordChannelId.isEmpty()) {
		StringBuffer errorMsg;
		errorMsg << "ERROR: Empty Discord channel ID for room: " << monitoredChatRoom->getFullPath();
		error(errorMsg);
		return;
	}

	auto botName = name;
	bot->on_message_create([botName, monitoredChatRoom, discordChannelId](const dpp::message_create_t & event) {
		auto channelId = event.msg.channel_id;
		if (channelId == dpp::snowflake(std::string(discordChannelId))) {

			// TODO: this is not a great way to do this, we should use bot id instead
			auto nameIsBotsName = String(event.msg.author.username) == botName;
			auto isBot = event.msg.author.is_bot();
			
			
			if (nameIsBotsName && isBot) {
				return;
			}

			if (!monitoredChatRoom) {
				return;
			}

			auto server = monitoredChatRoom->getZoneServer();
			if (server == nullptr) {
				return;
			}

			// use server nickname when available
			String discordUsername = event.msg.member.get_nickname();
			if (discordUsername.isEmpty()) {
				discordUsername = event.msg.author.username;
			}


			// set username and message colors to make it obvious what comes from discord
			String formattedDiscordUsername = "\\#ff3333" + discordUsername;
			String formattedDiscordMessage = " \\#ff66ff" + event.msg.content;
			ChatRoomMessage* msg = new ChatRoomMessage(formattedDiscordUsername, server->getGalaxyName(), formattedDiscordMessage, monitoredChatRoom->getRoomID());
			monitoredChatRoom->broadcastMessage(msg);
			
		} else {
			return; // not the channel we are monitoring, ignore
		}
	});
	
	StringBuffer successMsg;
	successMsg << "Successfully set up Discord monitoring for room: " << monitoredChatRoom->getFullPath();
	info(successMsg);
}

void DiscordBot::SendChatMessage(ChatRoom* room, const String senderName, const UnicodeString& msg) {
	if (bot == nullptr) {
		warning("SendChatMessage: bot is null - not configured on startup");
		return;
	}

	auto channelId = room->getDiscordChannelId();
	
	StringBuffer debugMsg;
	debugMsg << "SendChatMessage called - Room: " << room->getFullPath() << ", Sender: " << senderName << ", Channel ID: '" << channelId << "'";
	info(debugMsg);
	
	if (channelId.isEmpty()) {
		StringBuffer errorMsg;
		errorMsg << "ERROR: Empty Discord channel ID for room: " << room->getFullPath() << " - cannot send message to Discord";
		error(errorMsg);
		return;
	}

	StringBuffer formattedMessage;
	formattedMessage << "**" << senderName << ":** " << msg;
	
	StringBuffer sendMsg;
	sendMsg << "Sending message to Discord channel " << channelId << ": " << formattedMessage.toString();
	info(sendMsg);
	
	try {
		bot->message_create(dpp::message(dpp::snowflake(std::string(channelId)), std::string(formattedMessage.toString())));
		
		StringBuffer successMsg;
		successMsg << "Successfully sent message to Discord channel " << channelId;
		info(successMsg);
	} catch (const std::exception& e) {
		StringBuffer errorMsg;
		errorMsg << "ERROR: Failed to send message to Discord channel " << channelId << ": " << e.what();
		error(errorMsg);
	}
}

void DiscordBot::SendPvPLogMessage(CreatureObject* attacker, CreatureObject* defender) {
	if (bot == nullptr) {
		// do nothing, bot was not configured on startup
		return;
	}

	if (pvpLogChannelId.isEmpty()) {
		// do nothing, pvp channel is not configured, nowhere to send log
		return;
	}

	if (attacker == nullptr || defender == nullptr) {
		error("PvPLog: attack or defender was nullptr");
		return;
	}

	auto attackerGhost = attacker->getPlayerObject();
	auto defenderGhost = defender->getPlayerObject();
	if (attackerGhost == nullptr || defenderGhost == nullptr) {
		error("PvPLog: attacker or defender not valid player object");
		return;
	}

	StringBuffer description;

	auto attackerName = attacker->getFirstName();
	auto defenderName = defender->getFirstName();

	description << attackerName << " has bested " << defenderName;

	auto descriptionEnding = String(" in GCW combat.");
	String killType = "GCW";
	auto areInDuel = CombatManager::instance()->areInDuel(attacker, defender);
	if (areInDuel) {
		descriptionEnding = String(" in a duel.");
		killType = "Duel";
	}

	auto attackerHasMission = attacker->hasBountyMissionFor(defender);
	if (attackerHasMission) {
		descriptionEnding = String(", fulfilling their bounty mission.");
		killType = "Bounty";
	}

	auto defenderHasMission = defender->hasBountyMissionFor(attacker);
	if (defenderHasMission) {
		descriptionEnding = String(", preventing their attempt to collect a bounty.");
		killType = "Bounty";
	}

	description << descriptionEnding;

	auto attackerFaction = String("Unknown");
	switch (attacker->getFaction()) {
		case Factions::FACTIONNEUTRAL:
			attackerFaction = "Neutral";
			break;
		case Factions::FACTIONREBEL:
			attackerFaction = "Rebel";
			break;
		case Factions::FACTIONIMPERIAL:
			attackerFaction = "Imperial";
			break;
	}

	auto defenderFaction = String("Unknown");
	switch (defender->getFaction()) {
	case Factions::FACTIONNEUTRAL:
			defenderFaction = "Neutral";
			break;
	case Factions::FACTIONREBEL:
			defenderFaction = "Rebel";
			break;
	case Factions::FACTIONIMPERIAL:
			defenderFaction = "Imperial";
			break;
	}

	auto attackerAccountId = String::valueOf(attackerGhost->getAccountID());
	auto defenderAccountId = String::valueOf(defenderGhost->getAccountID());

	auto attackerGuildKey = String("None");
	auto attackerGuild = attacker->getGuildObject().get();
	if (attackerGuild != nullptr) {
			attackerGuildKey = attackerGuild->getGuildKey();
	}

	auto defenderGuildKey = String("None");
	auto defenderGuild = defender->getGuildObject().get();
	if (defenderGuild != nullptr) {
			defenderGuildKey = defenderGuild->getGuildKey();
	}

	auto embed = dpp::embed()
		.set_title(std::string("PvP Kill - " + killType))
		.set_description(std::string(description.toString()))
		.add_field("Attacker", std::string(attackerName + " (" + attackerAccountId + ")"), true)
		.add_field("Faction", std::string(attackerFaction), true)
		.add_field("Guild", std::string(attackerGuildKey), true)
		.add_field("Defender", std::string(defenderName + " (" + defenderAccountId + ")"), true)
		.add_field("Faction", std::string(defenderFaction), true)
		.add_field("Guild", std::string(defenderGuildKey), true)
		.set_timestamp(time(0));

	auto msg = dpp::message(dpp::snowflake(std::string(pvpLogChannelId)), embed);
	bot->message_create(msg);
}

void DiscordBot::HandleDppLogEvent(const dpp::log_t& log) {
	// translate DPP library logs to core3 logs
	switch (log.severity) {
		case dpp::loglevel::ll_trace:
		case dpp::loglevel::ll_debug:
			debug(log.message);
			break;
		case dpp::loglevel::ll_info:
			info(log.message);
			break;
		case dpp::loglevel::ll_warning:
			warning(log.message);
			break;
		case dpp::loglevel::ll_error:
		case dpp::loglevel::ll_critical:
			error(log.message);
			break;
		default:
			debug(log.message);
	}
}
#endif // WTH_DPP