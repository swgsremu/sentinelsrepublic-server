#!/usr/bin/env python3
"""
Test script to verify plugin system is receiving chat events via Redis
"""

import redis
import json
import time
import threading
from datetime import datetime

def monitor_redis_channels():
    """Monitor Redis channels for chat events"""
    r = redis.Redis(host='127.0.0.1', port=6379, decode_responses=True)
    pubsub = r.pubsub()
    
    # Subscribe to all CSR channels
    channels = ['csr:chat', 'csr:player_action', 'csr:admin_command']
    pubsub.subscribe(*channels)
    
    print(f"[{datetime.now()}] Monitoring Redis channels: {', '.join(channels)}")
    print("Waiting for events...")
    print("-" * 60)
    
    for message in pubsub.listen():
        if message['type'] == 'message':
            try:
                data = json.loads(message['data'])
                print(f"\n[{datetime.now()}] Channel: {message['channel']}")
                print(f"Event: {json.dumps(data, indent=2)}")
                print("-" * 60)
            except json.JSONDecodeError:
                print(f"\n[{datetime.now()}] Channel: {message['channel']}")
                print(f"Raw data: {message['data']}")
                print("-" * 60)

def test_chat_in_game():
    """Instructions for testing chat in game"""
    print("\n" + "=" * 60)
    print("PLUGIN SYSTEM TEST")
    print("=" * 60)
    print("\nTo test the plugin system:")
    print("1. Make sure the game server is running with plugin support")
    print("2. Log into the game")
    print("3. Try these chat commands:")
    print("   - /say Hello from plugin test")
    print("   - /tell [playername] Testing private messages")
    print("   - /group Testing group chat (must be in a group)")
    print("   - /guild Testing guild chat (must be in a guild)")
    print("\n4. You should see events appear below:")
    print("=" * 60 + "\n")

if __name__ == '__main__':
    # Show test instructions
    test_chat_in_game()
    
    # Start monitoring Redis
    try:
        monitor_redis_channels()
    except KeyboardInterrupt:
        print("\n\nStopping Redis monitor...")
    except Exception as e:
        print(f"\nError: {e}")
        print("\nMake sure Redis is running: sudo service redis-server start")