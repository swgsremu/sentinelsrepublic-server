/*
 * CSRCommandProcessor.h
 */

#ifndef CSRCOMMANDPROCESSOR_H_
#define CSRCOMMANDPROCESSOR_H_

#include "engine/engine.h"
#include "server/zone/ZoneServer.h"

// Forward declarations
namespace server {
namespace zone {
namespace objects {
namespace creature {
    class CreatureObject;
} // namespace creature
} // namespace objects
} // namespace zone
} // namespace server

namespace server {
namespace zone {
namespace managers {
namespace csr {

class CSRCommandProcessor : public Task, public Logger {
private:
    static CSRCommandProcessor* instance;
    Reference<ZoneServer*> zoneServer;
    bool isRunning;
    int pollInterval; // milliseconds
    
    void processPendingCommands();
    bool executeCommand(int commandId, const String& command, const String& parameters, const String& issuedBy);
    
    // Command implementations
    bool executeSystemMessage(const String& parameters, String& result);
    bool executeFreezePlayer(const String& parameters, String& result);
    bool executeUnfreezePlayer(const String& parameters, String& result);
    bool executeKickPlayer(const String& parameters, String& result);
    bool executeTeleportPlayer(const String& parameters, String& result);
    bool executeBroadcast(const String& parameters, String& result);
    bool executeGrantCredits(const String& parameters, String& result);
    
    void updateCommandStatus(int commandId, const String& status, const String& result);
    Reference<server::zone::objects::creature::CreatureObject*> findPlayerByName(const String& name);
    
public:
    CSRCommandProcessor(ZoneServer* server);
    ~CSRCommandProcessor();
    
    void run();
    void start();
    void stop();
    
    static CSRCommandProcessor* getInstance(ZoneServer* server = nullptr) {
        if (instance == nullptr && server != nullptr) {
            instance = new CSRCommandProcessor(server);
        }
        return instance;
    }
};

} // namespace csr
} // namespace managers
} // namespace zone
} // namespace server

using namespace server::zone::managers::csr;

#endif /* CSRCOMMANDPROCESSOR_H_ */