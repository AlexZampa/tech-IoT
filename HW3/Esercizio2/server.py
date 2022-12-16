import cherrypy
import json
import os.path

def load(file):
    if not os.path.exists(file):
        return []
    with open(file, "r") as f:
        log = json.load(f)
    return log

def dump(data, file):
    with open(file, "w") as f:
        json.dump(data, f)

class WebServer:
    exposed = True
    save_file = 'log.json'

    def GET(self, *uri, **params):
        log = load(self.save_file)
        return json.dumps(log)

    def POST(self, *uri, **params):
        d = json.loads(cherrypy.request.body.read())
        log = load(self.save_file)
        log.append(d)
        dump(log, self.save_file)

if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher()
            }
        }
    cherrypy.tree.mount(WebServer(), '/log', conf)
    cherrypy.config.update({ 'server.socket_host': '0.0.0.0' })
    cherrypy.config.update({ 'server.socket_port': 8080 })
    cherrypy.engine.start()
    cherrypy.engine.block()
