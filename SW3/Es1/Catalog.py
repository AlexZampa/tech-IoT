import json
import os

# Classe per gestire il catalogo (file json)
class Catalog:

    def __init__(self):
        self.path = os.path.abspath("./catalog.json")
        #JSON di default
        self.json = {
                "s": {
                    "R": {
                        "d": "192.168.100.126:8080/devices/subscription",
                        "s": "192.168.100.126:8080/services/subscription",
                        "u": "192.168.100.126:8080/users/subscription"
                    },
                    "M": {
                        "d": {
                            "h": "test.mosquitto.org",
                            "t": "/tiot/16/c/d/s"
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
