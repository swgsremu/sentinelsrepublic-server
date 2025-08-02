#ifndef REDISPUBLISHER_H_
#define REDISPUBLISHER_H_

#include <string>
#include <memory>
#include <hiredis/hiredis.h>

namespace server {
namespace zone {
namespace managers {
namespace redis {

class RedisPublisher {
private:
    std::string host;
    int port;
    std::string password;
    redisContext* context;
    bool connected;

    bool connect();
    void disconnect();
    
public:
    RedisPublisher(const std::string& host = "localhost", int port = 6379, const std::string& password = "");
    ~RedisPublisher();
    
    bool publishCommand(const std::string& commandId, const std::string& commandType, 
                       const std::string& status, const std::string& issuedBy);
    
    bool publishChatLog(int64_t senderOid, const std::string& senderName,
                       const std::string& message, const std::string& channelType,
                       const std::string& recipients);
                       
    bool publishAdminLog(const std::string& command, const std::string& issuer,
                        const std::string& target, const std::string& result);
                        
    bool publishPlayerLog(int64_t characterOid, const std::string& characterName,
                         const std::string& logType, const std::string& action,
                         const std::string& details);
                         
    bool publishNotification(const std::string& channel, const std::string& data);
    
    bool isConnected() const { return connected; }
    
    // Generic publish method
    bool publish(const std::string& channel, const std::string& message);
};

} // namespace redis
} // namespace managers
} // namespace zone
} // namespace server

#endif // REDISPUBLISHER_H_