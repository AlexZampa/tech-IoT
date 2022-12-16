from Devices import Devices
from Services import Services
from Users import Users
from Subscriptions import Subscriptions
from Catalog import Catalog
import cherrypy
from threading import Thread
import time

# Thread per l'eliminazione dei dispositivi e servizi ogni 60 secondi dal catalogo (classe Catalog)
class ClearThread(Thread):

    def __init__(self):
        Thread.__init__(self)

    def run(self):
        while True:
            time.sleep(60)
            print("Catalog clear")
            with Catalog() as c:
                c.set("devices", {
                    k: v
                    for k, v in c.get("devices").items()
                    if (time.time() - v["insert-timestamp"]) < 120
                })
                c.set("services", {
                    k: v
                    for k, v in c.get("services").items()
                    if time.time() - v["insert-timestamp"] < 120
                })

# main principale per creazione Catalog
# Tutte le richieste e risposte http vengono gestite dalle altri classi
# (una per le subscriptions, una per i services, una per i devices e una per gli users)
# la classe Catalog contiene la gestione del catalogo (file json)
def main():
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher()
            }
        }
    ClearThread().start()
    cherrypy.tree.mount(Subscriptions(), '/', conf)
    cherrypy.tree.mount(Devices(), '/devices', conf)
    cherrypy.tree.mount(Services(), '/services', conf)
    cherrypy.tree.mount(Users(), '/users', conf)
    cherrypy.config.update({'server.socket_host': '0.0.0.0' })
    cherrypy.config.update({'server.socket_port': 8080 })
    cherrypy.engine.start()
    cherrypy.engine.block()


if __name__ == '__main__':
    main()
