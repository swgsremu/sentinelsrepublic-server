#ifndef ADMINCOMMANDLOGGER_H_
#define ADMINCOMMANDLOGGER_H_

#include "engine/engine.h"
#include "server/zone/objects/creature/CreatureObject.h"

namespace server {
namespace zone {
namespace managers {
namespace admin {

class AdminCommandLogger : public Singleton<AdminCommandLogger>, public Logger, public Object {
public:
    AdminCommandLogger();
    ~AdminCommandLogger();
    
    void logCommand(CreatureObject* admin, const String& command, const String& arguments, bool success = true, const String& errorMessage = "");
    
private:
    void writeToDatabase(CreatureObject* admin, const String& command, const String& arguments, bool success, const String& errorMessage);
};

} // namespace admin
} // namespace managers
} // namespace zone
} // namespace server

#endif /* ADMINCOMMANDLOGGER_H_ */