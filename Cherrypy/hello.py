import cherrypy

class demoExample:
    def index(self):
        return "hello world"
    index.exposed = True

cherrypy.quickstart(demoExample())
