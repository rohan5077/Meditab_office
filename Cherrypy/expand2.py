import string
import random

import cherrypy

class expand2(object):
    @cherrypy.expose
    def index(self):
        return "Hello World"

    @cherrypy.expose
    def generate(self):
        return '-'.join(random.sample(string.hexdigits,8))

if __name__ == '__main__':
    cherrypy.quickstart(expand2())
