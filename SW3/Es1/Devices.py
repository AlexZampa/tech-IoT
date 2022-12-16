import cherrypy
import json
from Catalog import Catalog
import uuid
import time


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
            try:
                body = json.loads(cherrypy.request.body.read())
            except json.JSONDecodeError:
                raise cherrypy.HTTPError(422, "Error in json format")

            try:
                with Catalog() as c:
                    if "dID" in body:
                        deviceID = body["dID"]
                    else:
                        deviceID = str(uuid.uuid4())
                    c.get("devices")[deviceID] = {
                        "dID": deviceID,
                        "e": body["e"],
                        "r": body["r"],
                        "time": time.time()
                    }
                    return json.dumps({
                        "dID": deviceID,
                        "reg": "OK"
                    })
            except KeyError:
                raise cherrypy.HTTPError(422, "Error in data semantic")
        elif len(uri) == 2 and uri[0] == "subscription":
            with Catalog() as c:
                if uri[1] in c.get("devices"):
                    c.get("devices")[uri[1]]["time"] = time.time()
                    return json.dumps({
                        "dID": uri[1]
                    })
                else:
                    raise cherrypy.HTTPError(404, "Device not found")
        else:
            raise cherrypy.HTTPError(400, "Bad request")
