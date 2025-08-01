#ifndef CSRCOMMANDPROCESSOR_H_
#define CSRCOMMANDPROCESSOR_H_

#include "engine/engine.h"
#include "server/zone/ZoneServer.h"

namespace server {
namespace zone {
namespace managers {
namespace csr {

class CSRCommandProcessor : public Task, public Logger {
private:
    Reference<ZoneServer*> zoneServer;
    bool running;
    
public:
    CSRCommandProcessor(ZoneServer* server);
    ~CSRCommandProcessor();
    
    void run();
    void stop();
    
private:
    void processCommands();
    void executeCommand(const String& command, const String& parameters, const String& issuedBy, int commandId);
    void updateCommandStatus(int commandId, const String& status, const String& result);
    void logAdminCommand(const String& command, const String& parameters, const String& targetCharName, 
                        int64 targetCharOID, int targetAccID, const String& issuedBy, 
                        const String& result, const String& errorMsg);
    
    // Command implementations
    void freezePlayer(const String& playerName, const String& reason);
    void unfreezePlayer(const String& playerName);
    void kickPlayer(const String& playerName, const String& reason);
    void teleportPlayer(const String& playerName, float x, float y, float z, const String& planet);
    void sendSystemMessage(const String& playerName, const String& message);
    void sendBroadcast(const String& message);
    void grantCredits(const String& playerName, int amount);
};

} // namespace csr
} // namespace managers
} // namespace zone
} // namespace server

using namespace server::zone::managers::csr;

#endif /* CSRCOMMANDPROCESSOR_H_ */