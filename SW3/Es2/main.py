from MQTTClient import MQTTClient
import json
import requests
import time

def onReceived(paho_mqtt, userdata, msg):
    formatted = json.loads(msg.payload)
    print(f'Temperature :{formatted["e"][0]["v"]} {formatted["e"][0]["u"]}')


def main():
    ip_address = "192.168.100.126"
    r = requests.get(f"http://{ip_address}:8080/")
    url_sub = "http://" + json.loads(r.text)["R"]["s"]
    msg = {"e": {"R": {"r": "GET"}}, "d": "service that retrieve data sent by arduino"}

    # Subscription al Catalog come Service
    r = requests.post(url_sub, data=json.dumps(msg))
    serviceID = json.loads(r.text)["serviceID"]

    # Discovery
    r = requests.get(f"http://{ip_address}:8080/devices")
    devices = json.loads(r.text)

    # se sono presenti più device che registrano la temperatura, si sottoscrive a tutti
    arduino_devices = []
    for d in devices:
        if "t" in d["r"]["m"]:
            arduino_devices.append(d)

    client = MQTTClient("SubscriberMQTT", "test.mosquitto.org", 1883, onMessageReceived=onReceived)
    client.start()

    # si sottoscrive a più topic contemporaneamente (se necessario)
    topic = [(t, 2) for t in set([d["e"]["t"]["t"] for d in arduino_devices])]
    client.subscribe(topic)

    # aspetta di ricevere dati
    time.sleep(60)
    client.stop()


if __name__ == '__main__':
    main()
