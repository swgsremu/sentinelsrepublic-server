# Testing Guide for CSR Monitor Plugin

## Prerequisites
1. Ensure Redis is running: `sudo systemctl start redis-server`
2. Ensure the CSR monitor plugin is built: Check `/home/thomas/srswgemu2/MMOCoreORB/bin/plugins/csr-monitor.so`

## Testing Steps

### 1. Start the Server
Watch the console output for these key messages during startup:

```
[PluginLoader] Loading plugin: plugins/csr-monitor.so
[PluginLoader] Registering plugin as event listener and command handler
[EventDispatcher] Registered event listener: CSR Monitor Plugin Total listeners: 1
[PluginLoader] Successfully loaded plugin: CSR Monitor Plugin v1.0.0
[PluginLoader] Testing chat event dispatch to plugin
[EventDispatcher] dispatchChatEvent called with message: Test message from PluginLoader channel: TEST listeners: 1
[CSRMonitorPlugin] CSRMonitorPlugin::onChatEvent called - message: Test message from PluginLoader channel: TEST
[CSRMonitorPlugin] Received TEST chat event! Message: Test message from PluginLoader
```

If you see these messages, the plugin system is working correctly.

### 2. Test Chat Messages
1. Log into the game
2. Send various chat messages:
   - Spatial chat: Just type a message
   - Tell: `/tell playername message`
   - Group chat: `/g message` (while in a group)
   - Guild chat: `/gc message` (while in a guild)

For each message, you should see in the server console:
```
[ChatManager] ChatManager dispatching chat event - channel: SPATIAL message: your message
[EventDispatcher] dispatchChatEvent called with message: your message channel: SPATIAL listeners: 1
[CSRMonitorPlugin] CSRMonitorPlugin::onChatEvent called - message: your message channel: SPATIAL
```

### 3. Test Admin Commands
1. Use an admin character (level 15+)
2. Execute admin commands like:
   - `/setspeed 5`
   - `/teleport player 0 0 0`
   - `/kick playername`

You should see:
```
[CSRMonitorPlugin] CSRMonitorPlugin::onCommandEvent called - command: setspeed
```

### 4. Verify Redis
Check if events are being published to Redis:

```bash
redis-cli
> SUBSCRIBE csr:chat csr:admin_command csr:player_action
```

You should see JSON messages being published when events occur.

## Troubleshooting

### Plugin Not Loading
- Check if the .so file exists: `ls -la /home/thomas/srswgemu2/MMOCoreORB/bin/plugins/`
- Check for dlopen errors in server console

### Test Event Works but Real Chat Doesn't
- The plugin system is working but ChatManager integration has issues
- Check that all chat handlers are calling `dispatchChatEvent`

### No Events Reaching Plugin
- Check Redis connection in plugin
- Look for "Redis connection error" messages
- Ensure Redis is running: `redis-cli ping`

### Debug Output Not Visible
- The plugin logs to stdout/stderr
- Make sure you're watching the server console, not log files
- Debug messages use `[CSRMonitorPlugin]` prefix