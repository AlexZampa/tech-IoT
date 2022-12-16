
import json
from typing import Any, List, Tuple, Union
from MQTTClient import MQTTClient
from threading import Thread
from datetime import datetime

# mappa una funzione lineare dai dati in input
def map(x, in_min, in_max, out_min, out_max):
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min


class TemperatureRange():

    def __init__(self, low: float, high: float):
        self.low = low
        self.high = high

    def __init__(self, range: Tuple[float, float]):
        self.low = range[0]
        self.high = range[1]


# Classe che gestisce comandi SmartHomeController
class SmartHomeController():

    def __init__(self, 
    fan_temp: List[Union[TemperatureRange, Tuple[float, float]]],
    heat_temp: List[Union[TemperatureRange, Tuple[float, float]]],
    data_topic: str,
    command_topic: str,
    broker: str,
    port: int =1883):

        self.fan_temp = [ t if isinstance(t, TemperatureRange) else TemperatureRange(t) for t in fan_temp ]
        self.heat_temp = [ t if isinstance(t, TemperatureRange) else TemperatureRange(t) for t in heat_temp ]
        self.presence_pir = 0
        self.presence_sound = 0
        self.temp = 0
        self.fan_perc = 0
        self.heat_perc = 0
        self.lcd_message = ""
        self.client = MQTTClient("SmartHomeController", broker, port, onMessageReceived=self.dataReceived)
        self.data_topic = data_topic
        self.command_topic = command_topic
    
    def start(self):
        self.client.start()
        self.client.subscribe(self.data_topic)
    
    def dataReceived(self, paho_mqtt, userdata, msg):
        j = json.loads(msg.payload)
        self.presence_pir = j["e"][1]["v"]
        self.presence_sound = j["e"][2]["v"]
        self.temp = j["e"][0]["v"]
    
    def sendCommand(self, cmd: Any):
        self.client.publish(self.command_topic, json.dumps(cmd, separators=(',', ':')))
    
    def getPresence(self):
        return 1 if self.presence_pir or self.presence_sound else 0

    # controllo ventola
    def fan_control(self):
        if self.temp < self.fan_temp[self.getPresence()].low:
            self.fan_perc = 0
        elif self.temp > self.fan_temp[self.getPresence()].high:
            self.fan_perc = 100
        else:
            self.fan_perc = map(self.temp, self.fan_temp[self.getPresence()].low, self.fan_temp[self.getPresence()].high, 0, 100)
        return self.fan_perc

    # controllo led
    def heat_control(self):
        if self.temp < self.heat_temp[self.getPresence()].low:
            self.heat_perc = 100
        elif self.temp > self.heat_temp[self.getPresence()].high:
            self.heat_perc = 0
        else:
            self.fan_perc = map(self.temp, self.heat_temp[self.getPresence()].low, self.heat_temp[self.getPresence()].high, 100, 0)
        return self.heat_perc
    
    def loop(self):
        senML = {
            "bn": "Controller",
            "bt": datetime.now().microsecond,
            "e": [
                {
                    "n": "fan_speed",
                    "v": self.fan_control()
                },
                {
                    "n": "heat",
                    "v": self.heat_control()
                },
                {
                    "n": "message",
                    "v": self.lcd_message
                }
            ]
        }
        self.sendCommand(senML)
    
    def stop(self):
        self.client.stop()


class SmartHomeControllerThread(Thread):

    def __init__(self, controller: SmartHomeController):
        super().__init__()
        self.controller = controller
    
    def run(self):
        try:
            self.controller.start()
            while True:
                self.controller.loop()
        finally:
            self.controller.stop()
