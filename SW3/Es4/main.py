from SmartHomeController import *
from WebService import *
import json
import requests


def messageReceived(paho_mqtt, userdata, msg):
    formatted = json.dumps(json.loads(msg.payload), indent=2)
    print("Message Received")
    print(formatted)

# Main principale che gestisce WebService e SmartHomeController
# tramite richiese POST (gestite da classe WebService) si controlla la classe SmartHomeController,
# che invia comandi ad Arduino tramite MQTT
def main():
    ip = "192.168.1.6"
    r = requests.get(f"http://{ip}:8080/")
    j = json.loads(r.text)
    mqtt_endpoint = j["M"]["d"]

    # Subscription al Catalog come Service
    r = requests.post(j["R"]["s"], json.dumps({
        "e": {
            "R": {
                "s": {
                    "m": "GET",
                    "u": f"http://{ip}:8090/set_temp"
                },
                "g": {
                    "m": "GET",
                    "u": f"http://{ip}:8090/get_temp"
                }
            }
        },
        "d": "SmartHomeController service"
    }))

    # id di Arduino
    deviceID = "3fe07875-d1f4-47d1-ac81-0bac34593d21"
    r = requests.get(f"http://{ip}:8080/devices/{deviceID}")
    j = json.loads(r.text)
    # crea SmartHomeController
    controller = SmartHomeController([(27, 35), (25, 30)], [(15, 25), (15, 25)], j['e']['d'], j['e']['c'], mqtt_endpoint['h'])

    # lancia thread con controller
    ct = SmartHomeControllerThread(controller)
    ct.start()

    # lancia thread con WebService
    ws = WebServiceThread(controller)
    ws.start()

if __name__ == '__main__':
    main()
