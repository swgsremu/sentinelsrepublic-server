# Plugin System Testing Guide

## Prerequisites

1. **Build the game server** with plugin support:
   ```bash
   cd MMOCoreORB
   make -j$(nproc)
   ```

2. **Build the CSR monitor plugin**:
   ```bash
   cd plugins/csr-monitor
   mkdir -p build && cd build
   cmake ..
   make
   cp csr-monitor.so ../../../MMOCoreORB/bin/plugins/
   ```

3. **Start Redis**:
   ```bash
   sudo service redis-server start
   # or
   redis-server
   ```

## Testing Steps

### 1. Start the Test Monitor

In one terminal, run the Redis monitor:
```bash
./test-plugin-chat.py
```

This will show all events received through the plugin system.

### 2. Start the Game Server

In another terminal:
```bash
cd MMOCoreORB/bin
./core3
```

Look for these messages in the server log:
```
[PluginLoader] Loading plugins from: plugins
[PluginLoader] Loading plugin: plugins/csr-monitor.so
[PluginLoader] Successfully loaded plugin: CSR Monitor v1.0.0
[EventDispatcher] Registered event listener: CSR Monitor
```

### 3. Test Chat Events

Log into the game and test various chat types:

#### Spatial Chat
```
/say Hello from plugin test
/shout Testing shout
/yell Testing yell
```

#### Private Messages
```
/tell [playername] Testing private messages
```

#### Group Chat (must be in a group)
```
/group Testing group chat
```

#### Guild Chat (must be in a guild)
```
/guild Testing guild chat
```

#### Planet Chat
```
/planet Testing planet chat
```

#### Auction Chat
```
/auction Testing auction chat
```

### 4. Verify Events

In the Redis monitor terminal, you should see events like:
```json
{
  "timestamp": "2025-08-02T10:30:45",
  "type": "chat",
  "data": {
    "channelType": "SPATIAL",
    "senderName": "TestPlayer",
    "senderAccountID": 12345,
    "message": "Hello from plugin test",
    "recipientName": "",
    "planet": "tatooine",
    "posX": 3525.5,
    "posY": 4.2,
    "posZ": -4805.3
  }
}
```

### 5. Test Player Actions

Perform these actions to test player event monitoring:

- **Login/Logout**: Log in and out to see connect/disconnect events
- **Teleport**: Use `/teleport` or travel to see location change events
- **Zone**: Travel between planets

### 6. Test Admin Commands

If you have admin access, test command logging:
```
/setspeed 5
/teleport player 0 0 0
/kick player
```

## Troubleshooting

### Plugin Not Loading

1. Check the plugin path in config.lua:
   ```lua
   PluginDirectory = "plugins",
   EnablePlugins = true,
   ```

2. Verify the plugin file exists:
   ```bash
   ls -la MMOCoreORB/bin/plugins/csr-monitor.so
   ```

3. Check for loading errors in the server log

### No Events in Redis

1. Verify Redis is running:
   ```bash
   redis-cli ping
   # Should respond: PONG
   ```

2. Check plugin configuration in config.lua:
   ```lua
   CSRMonitor_RedisHost = "127.0.0.1",
   CSRMonitor_RedisPort = 6379,
   ```

3. Enable debug logging by modifying the plugin

### Chat Not Working

1. Ensure you're not muted in-game
2. Check that EventDispatcher is being called in ChatManager
3. Verify the plugin is registered as an event listener

## Debug Commands

### Check Redis Channels
```bash
redis-cli
> PUBSUB CHANNELS
# Should show: csr:chat, csr:player_action, csr:admin_command
```

### Monitor All Redis Activity
```bash
redis-cli MONITOR
```

### Check Plugin Status

The plugin logs its status to the game server console. Look for:
- Connection success/failure messages
- Event processing confirmations
- Batch flush notifications

## Performance Testing

To test the batching system:
1. Generate many chat messages quickly
2. Watch for "Flushing batch" messages in server log
3. Verify all messages appear in Redis

The plugin batches events and flushes every 5 seconds or when batch size reaches 100.