# -*- encoding=utf-8 -*-

# run this for debug: twistd -ny game.tac

try:
    from twisted.internet import epollreactor
    epollreactor.install()
except:
    pass

from twisted.application import internet, service
from twisted.web.websocket import WebSocketSite
from twisted.web import static
from carcassonne import GameHandler


wwwRoot = static.File('./res/www')
wwwRoot.indexNames = ['carcassonne.html']
site = WebSocketSite(wwwRoot)
site.addHandler("/ws/carcassonne", GameHandler)

application = service.Application('carcassonne')
internet.TCPServer(9876, site).setServiceParent(
    service.IServiceCollection(application))
