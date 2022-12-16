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
    ip_address = "192.168.100.126"
    r = requests.get(f"http://{ip_address}:8080/")
    j = json.loads(r.text)
    endpoint = j["M"]["d"]

    # Subscription al Catalog come Service
    r = requests.post("http://" + j["R"]["s"], json.dumps({
        "e": {
            "R": {
                "r": {
                    "m": "GET"
                }
            }
        },
        "d": "Simple service for turning on and off the led on the arduino"
    }))

    client = MQTTClient("Client", endpoint["h"], int(endpoint["p"]) if "p" in endpoint else 1883, onMessageReceived=messageReceived)
    client.start()

    a = 30
    while a > 0:
        # Discovery
        r = requests.get(f"http://{ip_address}:8080/devices")
        j = json.loads(r.text)
        for d in j:
            # controlla se tra i comandi del device esiste led
            if "l" in d["r"]["c"]:
                client.publish(d["e"]["l"]["t"], json.dumps({
                    "bn": "Yun",
                    "e": [
                        {
                            "n": "led",
                            "t": None,
                            "v": a % 2,
                            "u": None
                        }
                    ]
                }))
        time.sleep(15)
        a = a - 1
    
    client.stop()


if __name__ == '__main__':
    main()
