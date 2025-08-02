#!/usr/bin/env python3
"""
Test Redis subscriber to verify CSR monitor plugin is publishing events
"""

import redis
import json
import time
from datetime import datetime

def main():
    # Connect to Redis
    r = redis.Redis(host='localhost', port=6379, decode_responses=True)
    
    # Create pubsub
    pubsub = r.pubsub()
    
    # Subscribe to CSR channels
    channels = ['csr:chat', 'csr:player_action', 'csr:admin_command']
    pubsub.subscribe(*channels)
    
    print(f"[{datetime.now()}] Subscribed to channels: {', '.join(channels)}")
    print("Waiting for messages from CSR monitor plugin...")
    
    # Listen for messages
    for message in pubsub.listen():
        if message['type'] == 'message':
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            channel = message['channel']
            
            try:
                data = json.loads(message['data'])
                print(f"\n[{timestamp}] {channel}")
                print(json.dumps(data, indent=2))
            except json.JSONDecodeError:
                print(f"\n[{timestamp}] {channel}: {message['data']}")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\nShutting down...")
    except Exception as e:
        print(f"Error: {e}")