import cherrypy
import json
from Catalog import Catalog
import uuid

# Classe per la gestione dell'endpoint per gli utenti
class Users:
    exposed = True

    def GET(self, *uri, **query):
        if len(uri) == 1:
            with Catalog() as c:
                if uri[0] in c.get("users"):
                    return json.dumps(c.get("users")[uri[0]])
                raise cherrypy.HTTPError(404, "User not found")
        else:
            with Catalog() as c:
                return json.dumps([v for k, v in c.get("users").items()])
    
    def POST(self, *uri, **query):
        # Sottoscrizione
        if len(uri) == 1 and uri[0] == "subscription":
            try:
                body = json.loads(cherrypy.request.body.read())
            except json.JSONDecodeError:
                raise cherrypy.HTTPError(422, "Error in json format")

            try:
                with Catalog() as c:
                    userID = str(uuid.uuid4())
                    c.get("users")[userID] = {
                        "userID": userID,
                        "name": body["name"],
                        "surname": body["surname"],
                        "email": body["email"]
                    }
                    return json.dumps({
                        "userID": userID
                    })
            except KeyError:
                raise cherrypy.HTTPError(422, "Error in data semantic")
        else:
            raise cherrypy.HTTPError(400, "Bad request")
