from MQTTClient import MQTTClient
from Catalog import Catalog
from threading import Thread
import json
import uuid
import time

# Servizio per la gestione delle richieste MQTT
class MQTTService(Thread):

    def __init__(self):
        super().__init__()
        with Catalog() as c:
            self.hostname = c.get("subscriptions")["MQTT"]["device"]["hostname"]
            self.port = int(c.get("subscriptions")["MQTT"]["device"]["port"])
            self.base_topic = c.get("subscriptions")["MQTT"]["device"]["topic"]
        self.client = MQTTClient("CatalogServer", self.hostname, self.port, onMessageReceived=self.onReceived)

    def run(self):
        self.client.start()
        self.client.subscribe(self.base_topic)

    def onReceived(self, paho_mqtt, userdata, msg):
        print(msg.topic, msg.payload)
        if msg.topic == self.base_topic:
            try:
                body = json.loads(msg.payload)
                deviceID = body["deviceID"]
                with Catalog() as c:
                    if deviceID in c.get("devices"):
                        # Update Subscription
                        c.get("devices")[deviceID]["insert-timestamp"] = time.time()
                    else:
                        # Subscription
                        c.get("devices")[deviceID] = {
                            "deviceID": deviceID,
                            "end-points": body["end-points"],
                            "resources": body["resources"],
                            "insert-timestamp": time.time()
                        }

                self.client.publish(f"{self.base_topic}/{deviceID}",
                                    json.dumps({
                                        "deviceID": deviceID,
                                        "registered": "OK"
                                    }))
            except json.JSONDecodeError:
                print("ERROR decoding json ")
            except KeyError:
                print("ERROR accessing data")
