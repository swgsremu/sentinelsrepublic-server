#!/bin/bash

# Verify Plugin Build Integration Script

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Verifying Plugin Build Integration ===${NC}"
echo

# Check 1: Plugin headers exist
echo -e "${YELLOW}1. Checking plugin headers...${NC}"
PLUGIN_HEADERS=(
    "MMOCoreORB/src/server/zone/managers/plugin/PluginInterface.h"
    "MMOCoreORB/src/server/zone/managers/plugin/EventDispatcher.h"
    "MMOCoreORB/src/server/zone/managers/plugin/PluginLoader.h"
)

all_headers_found=true
for header in "${PLUGIN_HEADERS[@]}"; do
    if [ -f "${SCRIPT_DIR}/${header}" ]; then
        echo -e "${GREEN}✓ Found: ${header}${NC}"
    else
        echo -e "${RED}✗ Missing: ${header}${NC}"
        all_headers_found=false
    fi
done

if [ "$all_headers_found" = false ]; then
    echo -e "${RED}Some plugin headers are missing!${NC}"
    exit 1
fi

# Check 2: Plugin integration in core files
echo
echo -e "${YELLOW}2. Checking plugin integration...${NC}"

# Check ChatManager integration
if grep -q "dispatchChatEvent" "${SCRIPT_DIR}/MMOCoreORB/src/server/chat/ChatManager.idl"; then
    echo -e "${GREEN}✓ ChatManager.idl has dispatchChatEvent method${NC}"
else
    echo -e "${RED}✗ ChatManager.idl missing dispatchChatEvent method${NC}"
fi

if grep -q "EventDispatcher::instance()" "${SCRIPT_DIR}/MMOCoreORB/src/server/chat/ChatManagerImplementation.cpp"; then
    echo -e "${GREEN}✓ ChatManagerImplementation uses EventDispatcher${NC}"
else
    echo -e "${RED}✗ ChatManagerImplementation not using EventDispatcher${NC}"
fi

# Check ZoneServer integration
if grep -q "PluginLoader::instance()" "${SCRIPT_DIR}/MMOCoreORB/src/server/zone/ZoneServerImplementation.cpp"; then
    echo -e "${GREEN}✓ ZoneServer loads plugins on startup${NC}"
else
    echo -e "${RED}✗ ZoneServer not loading plugins${NC}"
fi

# Check 3: Build configuration
echo
echo -e "${YELLOW}3. Checking build configuration...${NC}"

# Check if plugin directory is in .gitignore
if grep -q "^plugins/" "${SCRIPT_DIR}/.gitignore"; then
    echo -e "${GREEN}✓ plugins/ directory is in .gitignore${NC}"
else
    echo -e "${YELLOW}⚠ plugins/ directory not in .gitignore${NC}"
fi

if grep -q "^MMOCoreORB/bin/plugins/" "${SCRIPT_DIR}/.gitignore"; then
    echo -e "${GREEN}✓ MMOCoreORB/bin/plugins/ is in .gitignore${NC}"
else
    echo -e "${YELLOW}⚠ MMOCoreORB/bin/plugins/ not in .gitignore${NC}"
fi

# Check 4: GitHub Actions filter
echo
echo -e "${YELLOW}4. Checking GitHub Actions filter...${NC}"

GITHUB_WORKFLOW="${SCRIPT_DIR}/.github/workflows/sync_filtered_to_public.yml"
if [ -f "$GITHUB_WORKFLOW" ]; then
    if grep -q "csr_command_processor\\.lua" "$GITHUB_WORKFLOW"; then
        echo -e "${GREEN}✓ GitHub Actions filters CSR files${NC}"
    else
        echo -e "${RED}✗ GitHub Actions not filtering CSR files${NC}"
    fi
else
    echo -e "${YELLOW}⚠ GitHub Actions workflow not found${NC}"
fi

# Summary
echo
echo -e "${BLUE}=== Build Instructions ===${NC}"
echo
echo "To build the server with plugin support:"
echo -e "${GREEN}1. Build the main server:${NC}"
echo "   cd ${SCRIPT_DIR}"
echo "   make build-ninja-debug"
echo
echo -e "${GREEN}2. Build the CSR plugin separately:${NC}"
echo "   cd ${SCRIPT_DIR}/plugins/csr-monitor"
echo "   ./build.sh"
echo
echo -e "${GREEN}3. Configure the server:${NC}"
echo "   Add to your server config:"
echo "   Core3.PluginDirectory = bin/plugins"
echo
echo -e "${YELLOW}Note:${NC} The plugin is built separately from the main server."
echo "This keeps the CSR monitoring code completely isolated."
echo
echo -e "${BLUE}The plugin headers are included in the main build automatically${NC}"
echo -e "${BLUE}through the GLOB_RECURSE in CMakeLists.txt, so the server will${NC}"
echo -e "${BLUE}compile successfully with make build-ninja-debug.${NC}"