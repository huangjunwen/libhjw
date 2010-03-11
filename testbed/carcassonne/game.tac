# -*- encoding=utf-8 -*-

# run this for debug: twistd -ny game.tac

import sys
platform = sys.platform

if platform.startswith('win'):
    from os.path import abspath
    sys.path.insert(0, abspath('.'))         # i don't know why, but without this won't work under windows
elif platform.startswith('linux'):
    try:
        from twisted.internet import epollreactor
        epollreactor.install()
    except:
        pass

from twisted.application import internet, service
from twisted.web.websocket import WebSocketSite
from twisted.web import static
from carcassonne import GameHandler

GameHandler.DEBUG = True

wwwRoot = static.File('./res/www')
wwwRoot.indexNames = ['carcassonne.html']
site = WebSocketSite(wwwRoot)
site.addHandler("/ws/carcassonne", GameHandler)

application = service.Application('carcassonne')
internet.TCPServer(9876, site).setServiceParent(
    service.IServiceCollection(application))
