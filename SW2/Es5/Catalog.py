import json
import os

# Classe per gestire il catalogo (file json)
class Catalog:

    def __init__(self):
        self.path = os.path.abspath("./catalog.json")
        #JSON di default
        self.json = {
                "subscriptions": {
                    "REST": {
                        "device": "http://192.168.100.126:8080/devices/subscription",
                        "service": "http://192.168.100.126:8080/serices/subscription",
                        "user": "http://192.168.100.126:8080/users/subscription"
                    },
                    "MQTT": {
                        "device": {
                            "hostname": "test.mosquitto.org",
                            "port": "1883",
                            "topic": "/tiot/16/catalog/devices/subscription"
                        }
                    }
                },
                "devices": {

                },
                "services": {

                },
                "users": {
                    
                }
            }
    
    def __enter__(self):
        if os.path.exists(self.path):
            with open(self.path, "r") as f:
                self.json = json.load(f)
        return self
    
    def get(self, attr):
        if attr in self.json:
            return self.json[attr]
        return None

    def set(self, attr, value):
        self.json[attr] = value

    def __exit__(self, type, value, traceback):
        with open(self.path, "w") as f:
            json.dump(self.json, f, indent=2)
