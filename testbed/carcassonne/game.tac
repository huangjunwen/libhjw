# -*- encoding=utf-8 -*-

# run this for debug: twistd -ny game.tac

import sys
from os.path import abspath

sys.path.insert(0, abspath('.'))                    # make sure curr dir in sys.path

if sys.platform.startswith('linux'):
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

wwwRoot = static.File('www')
wwwRoot.indexNames = ['carcassonne.html']
site = WebSocketSite(wwwRoot)
site.addHandler("/ws/carcassonne", GameHandler)

application = service.Application('carcassonne')
internet.TCPServer(9876, site).setServiceParent(
    service.IServiceCollection(application))
