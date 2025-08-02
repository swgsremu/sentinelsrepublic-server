#include "RedisPublisher.h"
#include <sstream>
#include <ctime>
#include <iomanip>
#include "server/zone/ZoneServer.h"

using namespace server::zone::managers::redis;

RedisPublisher::RedisPublisher(const std::string& host, int port, const std::string& password) 
    : host(host), port(port), password(password), context(nullptr), connected(false) {
    connect();
}

RedisPublisher::~RedisPublisher() {
    disconnect();
}

bool RedisPublisher::connect() {
    try {
        // Connect to Redis
        struct timeval timeout = { 1, 500000 }; // 1.5 seconds
        context = redisConnectWithTimeout(host.c_str(), port, timeout);
        
        if (context == nullptr || context->err) {
            if (context) {
                Logger::console.error() << "Redis connection error: " << context->errstr;
                redisFree(context);
                context = nullptr;
            } else {
                Logger::console.error() << "Redis connection error: can't allocate redis context";
            }
            connected = false;
            return false;
        }
        
        // Authenticate if password is provided
        if (!password.empty()) {
            redisReply* reply = (redisReply*)redisCommand(context, "AUTH %s", password.c_str());
            if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                if (reply) {
                    Logger::console.error() << "Redis authentication failed: " << reply->str;
                    freeReplyObject(reply);
                }
                disconnect();
                return false;
            }
            freeReplyObject(reply);
        }
        
        connected = true;
        Logger::console.info() << "Connected to Redis at " << host << ":" << port;
        return true;
        
    } catch (...) {
        Logger::console.error() << "Exception during Redis connection";
        connected = false;
        return false;
    }
}

void RedisPublisher::disconnect() {
    if (context) {
        redisFree(context);
        context = nullptr;
    }
    connected = false;
}

bool RedisPublisher::publish(const std::string& channel, const std::string& message) {
    if (!connected) {
        // Try to reconnect
        if (!connect()) {
            return false;
        }
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "PUBLISH %s %s", 
                                                   channel.c_str(), message.c_str());
    
    if (reply == nullptr) {
        // Connection lost, try to reconnect
        disconnect();
        if (!connect()) {
            return false;
        }
        // Retry the publish
        reply = (redisReply*)redisCommand(context, "PUBLISH %s %s", 
                                          channel.c_str(), message.c_str());
    }
    
    if (reply == nullptr) {
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReplyObject(reply);
    return success;
}

bool RedisPublisher::publishCommand(const std::string& commandId, const std::string& commandType,
                                   const std::string& status, const std::string& issuedBy) {
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // Create JSON message
    std::ostringstream json;
    json << "{"
         << "\"type\":\"command\","
         << "\"timestamp\":\"" << timestamp.str() << "\","
         << "\"data\":{"
         << "\"id\":\"" << commandId << "\","
         << "\"command_type\":\"" << commandType << "\","
         << "\"status\":\"" << status << "\","
         << "\"issued_by\":\"" << issuedBy << "\""
         << "}"
         << "}";
    
    return publish("csr:commands", json.str());
}

bool RedisPublisher::publishChatLog(int64_t senderOid, const std::string& senderName,
                                   const std::string& message, const std::string& channelType,
                                   const std::string& recipients) {
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // Escape message for JSON
    std::string escapedMessage = message;
    size_t pos = 0;
    while ((pos = escapedMessage.find("\"", pos)) != std::string::npos) {
        escapedMessage.replace(pos, 1, "\\\"");
        pos += 2;
    }
    
    // Create JSON message
    std::ostringstream json;
    json << "{"
         << "\"type\":\"chat_log\","
         << "\"timestamp\":\"" << timestamp.str() << "\","
         << "\"data\":{"
         << "\"sender_oid\":" << senderOid << ","
         << "\"sender_name\":\"" << senderName << "\","
         << "\"message\":\"" << escapedMessage << "\","
         << "\"channel_type\":\"" << channelType << "\"";
    
    if (!recipients.empty()) {
        json << ",\"recipients\":" << recipients;
    }
    
    json << "}"
         << "}";
    
    return publish("csr:logs:chat", json.str());
}

bool RedisPublisher::publishAdminLog(const std::string& command, const std::string& issuer,
                                    const std::string& target, const std::string& result) {
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // Create JSON message
    std::ostringstream json;
    json << "{"
         << "\"type\":\"admin_log\","
         << "\"timestamp\":\"" << timestamp.str() << "\","
         << "\"data\":{"
         << "\"command\":\"" << command << "\","
         << "\"issued_by\":\"" << issuer << "\","
         << "\"target\":\"" << target << "\","
         << "\"result\":\"" << result << "\","
         << "\"timestamp\":\"" << timestamp.str() << "\""
         << "}"
         << "}";
    
    return publish("csr:logs:admin", json.str());
}

bool RedisPublisher::publishPlayerLog(int64_t characterOid, const std::string& characterName,
                                     const std::string& logType, const std::string& action,
                                     const std::string& details) {
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // Create JSON message
    std::ostringstream json;
    json << "{"
         << "\"type\":\"player_log\","
         << "\"timestamp\":\"" << timestamp.str() << "\","
         << "\"data\":{"
         << "\"character_oid\":" << characterOid << ","
         << "\"character_name\":\"" << characterName << "\","
         << "\"log_type\":\"" << logType << "\","
         << "\"action\":\"" << action << "\","
         << "\"details\":" << details
         << "}"
         << "}";
    
    return publish("csr:logs:player", json.str());
}

bool RedisPublisher::publishNotification(const std::string& channel, const std::string& data) {
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // Create JSON message
    std::ostringstream json;
    json << "{"
         << "\"type\":\"notification\","
         << "\"timestamp\":\"" << timestamp.str() << "\","
         << "\"data\":" << data
         << "}";
    
    return publish("csr:events:" + channel, json.str());
}