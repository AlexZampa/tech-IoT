import paho.mqtt.client as PahoMQTT

# Classe per la gestione delle connessioni MQTT
class MQTTClient:
    def __init__(self,
                 clientID,
                 broker,
                 port,
                 notifier=None,
                 onConnect=None,
                 onMessageReceived=None):
        self.broker = broker
        self.port = port
        self.notifier = notifier
        self.clientID = clientID

        self._topic = ""
        self._isSubscriber = False

        self._paho_mqtt = PahoMQTT.Client(clientID, False)

        if onConnect is None:
            self._paho_mqtt.on_connect = self.myOnConnect
        else:
            self._paho_mqtt.on_connect = onConnect

        if onMessageReceived is None:
            self._paho_mqtt.on_message = self.myOnMessageReceived
        else:
            self._paho_mqtt.on_message = onMessageReceived

    def myOnConnect(self, paho_mqtt, userdata, flags, rc):
        print("Connected to %s with result code: %d" % (self.broker, rc))

    def myOnMessageReceived(self, paho_mqtt, userdata, msg):
        if self.notifier is not None:
            self.notifier.notify(msg.topic, msg.payload)

    def publish(self, topic, msg):
        self._paho_mqtt.publish(topic, msg, 2)

    def subscribe(self, topic):
        if self.notifier is not None:
            assert hasattr(self.notifier, 'notify')
        print("subscribing to %s" % (topic))

        # possibilità di iscriversi contemporaneamente a più topic
        if isinstance(topic, list):
            self._paho_mqtt.subscribe(topic)
            self._topic = [t for t,_ in topic]
        else:
            self._paho_mqtt.subscribe(topic, 2)
            self._topic = topic
        self._isSubscriber = True


    def start(self):
        self._paho_mqtt.connect(self.broker, self.port)
        self._paho_mqtt.loop_start()

    def unsubscribe(self):
        if self._isSubscriber:
            self._paho_mqtt.unsubscribe(self._topic)

    def stop(self):
        self.unsubscribe()
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()
