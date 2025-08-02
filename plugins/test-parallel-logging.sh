#!/bin/bash

# Test Script for Parallel Logging Migration
# This script helps verify that both the old CSR logging and new plugin system work in parallel

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CORE3_ROOT="${SCRIPT_DIR}/.."
PLUGIN_DIR="${SCRIPT_DIR}/csr-monitor"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== CSR Plugin Parallel Logging Test ===${NC}"
echo

# Step 1: Check if plugin exists
echo -e "${YELLOW}1. Checking plugin build...${NC}"
if [ ! -f "${PLUGIN_DIR}/build/csr-monitor.so" ]; then
    echo -e "${RED}Plugin not built. Building now...${NC}"
    cd "${PLUGIN_DIR}"
    ./build.sh
    cd "${SCRIPT_DIR}"
fi

if [ -f "${CORE3_ROOT}/MMOCoreORB/bin/plugins/csr-monitor.so" ]; then
    echo -e "${GREEN}✓ Plugin found in bin/plugins/${NC}"
else
    echo -e "${RED}✗ Plugin not found in bin/plugins/${NC}"
    exit 1
fi

# Step 2: Check database tables
echo
echo -e "${YELLOW}2. Checking database tables...${NC}"

# Create SQL to check tables
cat > /tmp/check_tables.sql << EOF
SELECT 
    'chat_logs' as table_name,
    COUNT(*) as old_count,
    MAX(timestamp) as latest_entry
FROM chat_logs
UNION ALL
SELECT 
    'admin_command_logs' as table_name,
    COUNT(*) as old_count,
    MAX(timestamp) as latest_entry
FROM admin_command_logs
UNION ALL
SELECT 
    'player_action_logs' as table_name,
    COUNT(*) as old_count,
    MAX(timestamp) as latest_entry
FROM player_action_logs;
EOF

echo -e "${BLUE}Current database state:${NC}"
mysql -h 127.0.0.1 -u swgemu -p123456 swgemu < /tmp/check_tables.sql

# Step 3: Monitor log files
echo
echo -e "${YELLOW}3. Setting up log monitoring...${NC}"

# Create monitoring script
cat > /tmp/monitor_logs.sh << 'EOF'
#!/bin/bash

# Monitor both old and new logging
echo "Monitoring logs for parallel logging verification..."
echo "Press Ctrl+C to stop"
echo

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Monitor function
monitor_database() {
    while true; do
        clear
        echo -e "${BLUE}=== Parallel Logging Monitor ===${NC}"
        echo -e "Time: $(date)"
        echo
        
        # Check chat logs
        echo -e "${YELLOW}Recent Chat Messages:${NC}"
        mysql -h 127.0.0.1 -u swgemu -p123456 swgemu -e "
            SELECT timestamp, sender_name, channel_type, 
                   LEFT(message, 50) as message_preview 
            FROM chat_logs 
            ORDER BY timestamp DESC 
            LIMIT 5;" 2>/dev/null
        
        echo
        echo -e "${YELLOW}Recent Admin Commands:${NC}"
        mysql -h 127.0.0.1 -u swgemu -p123456 swgemu -e "
            SELECT timestamp, admin_name, command, 
                   LEFT(arguments, 30) as args 
            FROM admin_command_logs 
            ORDER BY timestamp DESC 
            LIMIT 5;" 2>/dev/null
        
        echo
        echo -e "${YELLOW}Log Counts:${NC}"
        mysql -h 127.0.0.1 -u swgemu -p123456 swgemu -e "
            SELECT 
                (SELECT COUNT(*) FROM chat_logs WHERE timestamp > NOW() - INTERVAL 5 MINUTE) as chat_5min,
                (SELECT COUNT(*) FROM admin_command_logs WHERE timestamp > NOW() - INTERVAL 5 MINUTE) as cmd_5min,
                (SELECT COUNT(*) FROM player_action_logs WHERE timestamp > NOW() - INTERVAL 5 MINUTE) as action_5min;" 2>/dev/null
        
        sleep 5
    done
}

# Check for CSR plugin in server log
check_plugin_loaded() {
    if grep -q "CSR Monitor Plugin loaded" "${1}/MMOCoreORB/log/core3.log" 2>/dev/null; then
        echo -e "${GREEN}✓ CSR Monitor Plugin loaded successfully${NC}"
    else
        echo -e "${YELLOW}⚠ CSR Monitor Plugin not yet loaded${NC}"
    fi
}

# Start monitoring
monitor_database
EOF

chmod +x /tmp/monitor_logs.sh

# Step 4: Test commands
echo
echo -e "${YELLOW}4. Test Commands:${NC}"
echo
echo -e "${BLUE}To test parallel logging:${NC}"
echo "1. Start the server with the plugin enabled"
echo "2. Run the monitoring script: /tmp/monitor_logs.sh"
echo "3. In game, test these actions:"
echo "   - Send chat messages in different channels"
echo "   - Execute admin commands"
echo "   - Perform player actions (login, teleport, etc.)"
echo
echo -e "${BLUE}To verify plugin status in-game:${NC}"
echo "   /csrmonitorstatus"
echo
echo -e "${BLUE}Configuration to add to your server config:${NC}"
cat << EOF
Core3.PluginDirectory = bin/plugins
CSRMonitor.LogChat = true
CSRMonitor.LogActions = true
CSRMonitor.LogCommands = true
EOF

echo
echo -e "${YELLOW}5. Checking for conflicts...${NC}"

# Check if old CSR code is still active
if grep -q "logChatMessage" "${CORE3_ROOT}/MMOCoreORB/src/server/chat/ChatManagerImplementation.cpp" 2>/dev/null; then
    echo -e "${YELLOW}⚠ Old CSR logging code still present in ChatManager${NC}"
    echo "  This is expected during parallel testing phase"
else
    echo -e "${GREEN}✓ Old CSR logging code has been removed${NC}"
fi

# Check event dispatcher
if grep -q "dispatchChatEvent" "${CORE3_ROOT}/MMOCoreORB/src/server/chat/ChatManagerImplementation.cpp" 2>/dev/null; then
    echo -e "${GREEN}✓ Event dispatcher integration found${NC}"
else
    echo -e "${RED}✗ Event dispatcher integration missing${NC}"
fi

echo
echo -e "${GREEN}Test setup complete!${NC}"
echo "Run /tmp/monitor_logs.sh to start monitoring parallel logging"