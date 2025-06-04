#ifndef DISCORDBOT_H
#define DISCORDBOT_H

#ifdef WITH_DPP

#include <dpp/cluster.h>
// Removed "using namespace dpp;" to prevent namespace pollution causing socket() ambiguity

#include "server/chat/room/ChatRoom.h"
#include "engine/util/Singleton.h"
#include "system/lang/Object.h"
#include "engine/log/Logger.h"
#include "server/zone/objects/creature/CreatureObject.h"

namespace server { namespace discord {

class DiscordBot : public Singleton<DiscordBot>, public Object, public Logger {
public:
	DiscordBot();
	~DiscordBot();

	void InitializeBot(String botName, String botToken);

	// Start the dpp bot in the background
	void Start();
	void Shutdown();

	// Add a game chatroom to be synchronized with a discord chat channel
	void AddMonitoredChatRoom(ChatRoom* room);

	// Send a chat message from a ChatRoom to discord
	void SendChatMessage(ChatRoom* room, const String senderName, const UnicodeString& message);

	void SendPvPLogMessage(CreatureObject* attacker, CreatureObject* defender);


	void HandleDppLogEvent(const dpp::log_t& log);


private:
	dpp::cluster* bot = nullptr;
	String name;
	String pvpLogChannelId;
};

} }

using namespace server::discord;

#else // !WITH_DPP

namespace server { namespace discord {
// Dummy DiscordBot class for builds without DPP
class DiscordBot : public Object {
public:
    void release() {}
    void Start() {}
    void Shutdown() {}
    void InitializeBot(String, String) {}
    void AddMonitoredChatRoom(ChatRoom*) {}
    void SendChatMessage(ChatRoom*, const String, const UnicodeString&) {}
    void SendPvPLogMessage(CreatureObject*, CreatureObject*) {}
};
} }
using namespace server::discord;

#endif // WITH_DPP

#endif // DISCORDBOT_H
