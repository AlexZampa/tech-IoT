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

    def PUT(self, *uri, **query):
        body = json.loads(cherrypy.request.body.read())
        if not all(k in body for k in ("values", "originalUnit", "targetUnit")):
            raise cherrypy.HTTPError(400, "Missing parameters")
        if not all(type(v) in (int, float) for v in body["values"]):
            raise cherrypy.HTTPError(400, "Value must be a number")
        if body["originalUnit"] not in [ "K", "F", "C" ]:
            raise cherrypy.HTTPError(400, "originalUnit must be Kelvin, Celsius or Fahrenheit")
        if body["targetUnit"] not in [ "K", "F", "C" ]:
            raise cherrypy.HTTPError(400, "targetUnit must be Kelvin, Celsius or Fahrenheit")
        body["values"] = [ float(v) for v in body["values"] ]
        return json.dumps({
            "originalValues": body["values"],
            "originalUnit": body["originalUnit"],
            "targetValues": [ self.converter[body["originalUnit"]][body["targetUnit"]](v) for v in body["values"]],
            "targetUnit": body["targetUnit"]
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
