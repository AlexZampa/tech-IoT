from MQTTClient import MQTTClient
from Catalog import Catalog
from threading import Thread
import json
import uuid
import time


class MQTTService(Thread):

    def __init__(self):
        super().__init__()
        with Catalog() as c:
            self.hostname = c.get("s")["M"]["d"]["h"]
            self.port = 1883
            self.base_topic = c.get("s")["M"]["d"]["t"]
            print(self.base_topic)
        self.client = MQTTClient("CatalogServer", self.hostname, self.port, onMessageReceived=self.onReceived)

    def run(self):
        self.client.start()
        self.client.subscribe(self.base_topic)

    def onReceived(self, paho_mqtt, userdata, msg):
        print(msg.topic, msg.payload)
        if msg.topic == self.base_topic:
            try:
                body = json.loads(msg.payload)
                deviceID = body["dID"]
                with Catalog() as c:
                    if deviceID in c.get("devices"):
                        c.get("devices")[deviceID]["time"] = time.time()
                    else:
                        c.get("devices")[deviceID] = {
                            "dID": deviceID,
                            "e": body["e"],
                            "r": body["r"],
                            "time": time.time()
                        }
                print(f"deviceID = {deviceID}")
                print(f"topic = {self.base_topic}/{deviceID}")
                self.client.publish(f"{self.base_topic}/{deviceID}",
                                    json.dumps({
                                        "dID": deviceID,
                                        "reg": "OK"
                                    }, separators=(',', ':')))
            except json.JSONDecodeError:
                print("ERROR decoding json ")
            except KeyError:
                print("ERROR accessing data")
