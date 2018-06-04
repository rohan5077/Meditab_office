import cherrypy

class HelloWorld(object):
    @cherrypy.expose
    def index(self):
        return "Hello Google chrome"

if __name__ == '__main__':
    cherrypy.quickstart(HelloWorld())
