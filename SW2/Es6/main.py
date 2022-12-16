from MQTTClient import MQTTClient
import json
import uuid
import requests
import time

def messageReceived(paho_mqtt, userdata, msg):
    formatted = json.dumps(json.loads(msg.payload), indent=2)
    print("Message Received")
    print(formatted)

def main():
    ip_address = "192.168.1.6"
    r = requests.get(f"http://{ip_address}:8080/")
    j = json.loads(r.text)
    endpoint = j["subscriptions"]["MQTT"]["device"]
    client = MQTTClient("Client", endpoint["hostname"], int(endpoint["port"]), onMessageReceived=messageReceived)
    deviceID = str(uuid.uuid4())
    client.start()
    client.subscribe(f"{endpoint['topic']}/{deviceID}")

    a = 0
    while a < 30:
        client.publish(f"{endpoint['topic']}", json.dumps({
            "deviceID": deviceID,
            "end-points": [
                "/test/device"
            ],
            "resources": [
                "Temperature",
                "Humidity"
            ]
        }))
        time.sleep(10)
    client.stop()


if __name__ == '__main__':
    main()
