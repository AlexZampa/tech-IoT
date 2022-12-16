import cherrypy
import json
from Catalog import Catalog
import uuid

class Subscriptions:
    exposed = True

    def GET(self, *uri, **query):
        if len(uri) == 0:
            with Catalog() as c:
                # print(c.get("s"))
                return json.dumps(c.get("s"), separators=(',', ':'))
