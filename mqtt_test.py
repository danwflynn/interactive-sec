import paho.mqtt.client as mqtt
import time

# MQTT broker details
BROKER = "localhost"  # Replace with the MQTT broker address
PORT = 1883           # Default MQTT port
TOPIC = "test/topic"  # Replace with your test topic

# The message to publish
TEST_MESSAGE = "Hello, MQTT!"

# Callback for when the client receives a connection acknowledgment
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker successfully!")
        client.subscribe(TOPIC)
    else:
        print(f"Failed to connect, return code {rc}")

# Callback for when a message is received
def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}: {msg.payload.decode()}")
    # Stop the loop after receiving the test message
    if msg.payload.decode() == TEST_MESSAGE:
        client.loop_stop()

# Initialize MQTT client
client = mqtt.Client()

# Assign callbacks
client.on_connect = on_connect
client.on_message = on_message

# Connect to the broker
print("Connecting to broker...")
client.connect(BROKER, PORT, 60)

# Start the client loop in a separate thread
client.loop_start()

# Wait briefly to ensure subscription
time.sleep(1)

# Publish a test message
print(f"Publishing message to topic '{TOPIC}': {TEST_MESSAGE}")
client.publish(TOPIC, TEST_MESSAGE)

# Wait for the message to be received
time.sleep(5)

# Disconnect the client
print("Disconnecting from broker...")
client.loop_stop()
client.disconnect()

print("Test complete.")
