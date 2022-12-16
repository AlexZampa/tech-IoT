import json
import os

# Classe per gestire il catalogo
# ogni operazione sul catalogo implica aprire il file catalog.json, modificarlo (o leggerlo) e chiuderlo
# la classe Catalog può essere usata con la keyword with
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
                            "hostname": "iot.eclipse.org",
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
    
    #Per usare la classe con with
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

    #Per usare la classe con with
    def __exit__(self, type, value, traceback):
        with open(self.path, "w") as f:
            json.dump(self.json, f, indent=2)
