import json
from Catalog import Catalog

# Classe per la gestione dell'endpoint per le sottoscrizioni
class Subscriptions:
    exposed = True

    def GET(self, *uri, **query):
        if len(uri) == 0:
            with Catalog() as c:
                return json.dumps({
                    "subscriptions": c.get("subscriptions")
                })
