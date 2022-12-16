import json
from SmartHomeController import SmartHomeController
from threading import Thread
import cherrypy

class Webservice():

    def __init__(self, controller: SmartHomeController):
        self.controller = controller

    @cherrypy.expose
    def POST(self, *uri, **query):
        #riceve i comandi via POST
        if len(uri) == 1 and uri[1] == "set_temp":
            body = json.loads(cherrypy.request.body.read())

            if "mode" not in body or "presence" not in body or "end-point" not in body or "temperature" not in body:
                raise cherrypy.HTTPError(400, "Missing argument")

            if body["mode"] not in [ "fan", "heat" ]:
                raise cherrypy.HTTPError(400, "Mode not valid")
            if body["presence"] not in [ 0, 1 ]:
                raise cherrypy.HTTPError(400, "Presence not valid")
            if body["end-point"] not in [ "low", "high" ]:
                raise cherrypy.HTTPError(400, "End-point not valid")

            if body["mode"] == "fan":
                if body["end-point"] == "low":
                    self.controller.fan_temp[body["presence"]].low = body["temperature"]
                else:
                    self.controller.fan_temp[body["presence"]].high = body["temperature"]
            else:
                if body["end-point"] == "low":
                    self.controller.heat_temp[body["presence"]].low = body["temperature"]
                else:
                    self.controller.heat_temp[body["presence"]].high = body["temperature"]
            return json.dumps({
                "fan_temp": self.controller.fan_temp,
                "heat_temp": self.controller.heat_temp
            })
        elif len(uri) == 1 and uri[1] == "message":
            body = json.loads(cherrypy.request.body.read())
            if "message" not in body:
                raise cherrypy.HTTPError(400, "Missing message argument")
            self.controller.lcd_message = body["message"]
        raise cherrypy.HTTPError(404, "Not found")
        
    @cherrypy.expose
    def GET(self, *uri, **query):
        if len(uri) == 1 and uri[1] == "get_temp":
            return json.dumps({
                "fan_temp": self.controller.fan_temp,
                "heat_temp": self.controller.heat_temp
            })
        raise cherrypy.HTTPError(404, "Not found")


class WebServiceThread(Thread):

    def __init__(self, controller: SmartHomeController):
        super().__init__()
        self.controller = controller
    
    def run(self):
        try:
            conf = {
            '/': {
                'request.dispatch': cherrypy.dispatch.MethodDispatcher()
                }
            }
            cherrypy.tree.mount(Webservice(self.controller), '/', conf)
            cherrypy.config.update({'server.socket_host': '0.0.0.0' })
            cherrypy.config.update({'server.socket_port': 8090 })
            cherrypy.engine.start()
            cherrypy.engine.block()
        except Exception as e:
            print(e.with_traceback())
