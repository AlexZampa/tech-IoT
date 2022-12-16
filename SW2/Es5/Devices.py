import cherrypy
import json
from Catalog import Catalog
import uuid
import time

# Classe per la gestione dell'endpoint per i dispositivi
class Devices:
    exposed = True

    def GET(self, *uri, **query):
        if len(uri) == 1:
            with Catalog() as c:
                if uri[0] in c.get("devices"):
                    return json.dumps(c.get("devices")[uri[0]])
                raise cherrypy.HTTPError(404, "Device not found")
        else:
            with Catalog() as c:
                return json.dumps([v for k, v in c.get("devices").items()])

    def POST(self, *uri, **query):
        if len(uri) == 1 and uri[0] == "subscription":
            # Subscription
            try:
                body = json.loads(cherrypy.request.body.read())
            except json.JSONDecodeError:
                raise cherrypy.HTTPError(422, "Error in json format")

            try:
                with Catalog() as c:
                    deviceID = str(uuid.uuid4())
                    c.get("devices")[deviceID] = {
                        "deviceID": deviceID,
                        "end-points": body["end-points"],
                        "resources": body["resources"],
                        "insert-timestamp": time.time()
                    }
                    return json.dumps({
                        "deviceID": deviceID
                    })
            except KeyError:
                raise cherrypy.HTTPError(422, "Error in data semantic")
        elif len(uri) == 2 and uri[0] == "subscription":
            # Update subscription
            with Catalog() as c:
                if uri[1] in c.get("devices"):
                    c.get("devices")[uri[1]]["insert-timestamp"] = time.time()
                    return json.dumps({
                        "deviceID": uri[1]
                    })
                else:
                    raise cherrypy.HTTPError(404, "Device not found")
        else:
            raise cherrypy.HTTPError(400, "Bad request")
