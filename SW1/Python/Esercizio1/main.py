import cherrypy
import re
import json

class TemperatureConverter:
    exposed = True

    converter = {
        'K': {
            'K': lambda x: x,
            'C': lambda x: x - 273.15,
            'F': lambda x: (x - 273.15) * 9 / 5 + 32
        },
        'C': {
            'K': lambda x: x + 273.15,
            'C': lambda x: x,
            'F': lambda x: x * 9 / 5 + 32
        },
        'F': {
            'K': lambda x: (x - 32) * 5 / 9 + 273.15,
            'C': lambda x: (x - 32) * 5 / 9,
            'F': lambda x: x
        }
    }

    def GET(self, *uri, **query):
        if not all(k in query for k in ("value", "originalUnit", "targetUnit")):
            raise cherrypy.HTTPError(400, "Missing parameters")
        # regexp per controllare se stringa è numero (anche virgola e segno negativo)
        if re.match(r'^-?\d+(\.\d+)?$', query["value"]) is None:
            raise cherrypy.HTTPError(400, "Value must be a number")
        if query["originalUnit"] not in [ "K", "F", "C" ]:
            raise cherrypy.HTTPError(400, "originalUnit must be Kelvin, Celsius or Fahrenheit")
        if query["targetUnit"] not in [ "K", "F", "C" ]:
            raise cherrypy.HTTPError(400, "targetUnit must be Kelvin, Celsius or Fahrenheit")
        query["value"] = float(query["value"])
        return json.dumps({
            "originalValue": query["value"],
            "originalUnit": query["originalUnit"],
            "targetValue": self.converter[query["originalUnit"]][query["targetUnit"]](query["value"]),
            "targetUnit": query["targetUnit"]
        })
            

if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            }
        }
    cherrypy.tree.mount(TemperatureConverter(), '/converter', conf)
    cherrypy.config.update({ 'server.socket_host': '0.0.0.0' })
    cherrypy.config.update({ 'server.socket_port': 8080 })
    cherrypy.engine.start()
    cherrypy.engine.block()
