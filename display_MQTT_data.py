# File to be runned on Linux device to display MQTT data by subscribing to the topic

import paho.mqtt.client as mqtt

#  MQTT Broker Details
BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "vaibhav/esp32/data"

#  Callback when connected
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker")
        client.subscribe(TOPIC)
    else:
        print("Connection failed with code", rc)

#  Callback when message is received
def on_message(client, userdata, msg):
    print(f"Received: {msg.payload.decode()}")

#  Create client
client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message

#  Connect to broker
client.connect(BROKER, PORT, 60)

#  Start loop
client.loop_forever()
