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
        if len(uri) != 3:
            raise cherrypy.HTTPError(400, "Missing parameters")
        # regexp per controllare se stringa è numero (anche virgola e segno negativo)
        if re.match(r'^-?\d+(\.\d+)?$', uri[0]) is None:
            raise cherrypy.HTTPError(400, "Value must be a number")
        if uri[1] not in [ "K", "F", "C" ]:
            raise cherrypy.HTTPError(400, "originalUnit must be Kelvin, Celsius or Fahrenheit")
        if uri[2] not in [ "K", "F", "C" ]:
            raise cherrypy.HTTPError(400, "targetUnit must be Kelvin, Celsius or Fahrenheit")
        val = float(uri[0])
        return json.dumps({
            "originalValue": val,
            "originalUnit": uri[1],
            "targetValue": self.converter[uri[1]][uri[2]](val),
            "targetUnit": uri[2]
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
