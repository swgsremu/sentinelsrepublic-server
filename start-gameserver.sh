#!/bin/bash
# Start SWGEmu game server with CSR integration

# Set Redis connection for CSR plugin
export CSR_REDIS_HOST="localhost"
export CSR_REDIS_PORT="6379"
export CSR_REDIS_PASSWORD="srswgemu-redis-pass"  # Change this to match your web UI

# Start the game server
cd /home/thomas/srswgemu2/MMOCoreORB/bin
./core3