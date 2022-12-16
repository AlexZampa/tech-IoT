import cherrypy
import json
from Catalog import Catalog
import uuid
import time

class Services:
    exposed = True

    def GET(self, *uri, **query):
        if len(uri) == 1:
            with Catalog() as c:
                if uri[0] in c.get("services"):
                    return json.dumps(c.get("services")[uri[0]])
                raise cherrypy.HTTPError(404, "Service not found")
        else:
            with Catalog() as c:
                return json.dumps([v for k, v in c.get("services").items()])
    
    def POST(self, *uri, **query):
        if len(uri) == 1 and uri[0] == "subscription":
            try:
                body = json.loads(cherrypy.request.body.read())
            except json.JSONDecodeError:
                raise cherrypy.HTTPError(422, "Error in json format")

            try:
                with Catalog() as c:
                    serviceID = str(uuid.uuid4())
                    c.get("services")[serviceID] = {
                        "serviceID": serviceID,
                        "e": body["e"],
                        "d": body["d"],
                        "insert-timestamp": time.time()
                    }
                    return json.dumps({
                        "serviceID": serviceID
                    })
            except KeyError:
                raise cherrypy.HTTPError(422, "Error in data semantic")
        else:
            raise cherrypy.HTTPError(400, "Bad request")
