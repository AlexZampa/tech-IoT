import cherrypy
import os
import json

class Freeboard:
    exposed = True

    def GET(self, *uri, **query):
        return open("./freeboard/index.html")
    
    def POST(self, *uri, **query):
        body = json.loads(cherrypy.request.body.params['json_string'])
        with open('./freeboard/dashboard/dashboard.json', 'w') as f:
            f.write(json.dumps(body, indent=4))

if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.staticdir.root': os.path.abspath('./freeboard')
            },
        '/css': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'css'
            },
        '/dashboard': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'dashboard'
            },
        '/img': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'img'
            },
        '/js': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'js'
            },
        '/plugins': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'plugins'
            }
        }
    cherrypy.tree.mount(Freeboard(), '/', conf)
    cherrypy.config.update({ 'server.socket_host': '0.0.0.0' })
    cherrypy.config.update({ 'server.socket_port': 8080 })
    cherrypy.engine.start()
    cherrypy.engine.block()
