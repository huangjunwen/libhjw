
# twistd -ny game.tac

try:
    from twisted.internet import epollreactor
    epollreactor.install()
except:
    pass

from twisted.python import log
from twisted.application import internet, service
from twisted.web.websocket import WebSocketSite
from twisted.web.resource import Resource
from twisted.web import static

from wsjson import *


application = service.Application('carcassonne')
wwwRoot = static.File('./res/www')
wwwRoot.indexNames = ['carcassonne.html']
site = WebSocketSite(wwwRoot)
internet.TCPServer(9876, site).setServiceParent(
    service.IServiceCollection(application))
