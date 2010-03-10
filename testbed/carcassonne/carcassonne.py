# -*- encoding=utf-8 -*-

import re
from twisted.python import log
from wsjson import *
from event import EventSrc

class Player(object):
    
    create_cnt = 0

    def __init__(self, nickname, handler):
        self.id = "P%d" % Player.create_cnt
        Player.create_cnt += 1
        self.nickname = nickname
        self.handler = handler

        # manuplate by Room
        self.game = None
        self.color = None
        self.ready = False

    def notify(self, method, params):
        self.handler.notify(method, params)


GAME_ST_IDLE = 0

GAME_ST_GAMING = 1

MIN_PLAYER_PER_GAME = 2

MAX_PLAYER_PER_GAME = 5


class Game(EventSrc):

    def __init__(self, id):
        super(Game, self).__init__()
        self.id = id
        self.st = GAME_ST_IDLE 

        # players dict
        self.id2player = {}
        self.nick2player = {}
        self.color2player = {}

        self.readyCnt = 0

        # a notifier to broadcast events
        self.notifier = GameNotifier(self)

    def players(self):
        for p in self.id2player.itervalues():
            yield p
    
    def join(self, player):
        if self.st == GAME_ST_GAMING:
            return False, "游戏中, 暂时无法加入"

        if len(self.id2player) >= MAX_PLAYER_PER_GAME:
            return False, "人满了"

        if player.id in self.id2player:
            return False, "你已经在房间中了"

        if player.nickname in self.nick2player:
            return False, "重名了"

        # leave the orignal game
        if player.game:
            player.game.leave(player)

        self.id2player[player.id] = player
        self.nick2player[player.nickname] = player
        for color in xrange(MAX_PLAYER_PER_GAME):
            if color not in self.color2player:
                break
        self.color2player[color] = player

        player.game = self
        player.color = color
        player.ready = False
        self.fireEv('join', player=player)
        return True, ""

    def leave(self, player):
        if player.id not in self.id2player:
            return False

        # XXX some clean task here
        if self.st == GAME_ST_GAMING:
            pass                                            

        # remove from game
        p = self.id2player.pop(player.id)
        assert p == player
        p = self.nick2player.pop(player.nickname)
        assert p == player
        p = self.color2player.pop(player.color)
        assert p == player
        player.room = None
        player.color = None
        if player.ready:
            self.readyCnt -= 1
            player.ready = False
        self.fireEv('leave', player=player)

        self._chkGameStart()
        return True

    def chat(self, player, msg):    
        if player.id not in self.id2player:
            return False
        self.fireEv('chat', player=player, msg=msg)

    def ready(self, player):
        if player.id not in self.id2player:
            return False

        if self.st != GAME_ST_IDLE or player.ready:
            return False

        player.ready = True
        self.readyCnt += 1
        self.fireEv('ready', player=player)
        self._chkGameStart()

        
    def _chkGameStart(self):
        if self.readyCnt >= MIN_PLAYER_PER_GAME and \
                self.readyCnt != len(self.id2player):
            return
        # XXX start game
        log.msg("all players ready")


class GameNotifier(object):
    
    def __init__(self, game):
        self.game = game
        game.addEvListener('join', self.onJoin)
        game.addEvListener('leave', self.onLeave)
        game.addEvListener('chat', self.onChat)
        game.addEvListener('ready', self.onReady)

    def notifyAll(self, method, *params):
        for p in self.game.players():
            p.notify(method, params)

    def notifyAllExcept(self, expt, method, *params):
        for p in self.game.players():    
            if p is expt:
                continue
            p.notify(method, params)

    def onJoin(self, ev):
        player = ev.player
        self.notifyAllExcept(player, 'join', player.id, player.nickname, player.color)

    def onLeave(self, ev):
        self.notifyAll('leave', ev.player.id)

    def onChat(self, ev):   
        self.notifyAllExcept(ev.player, 'chat', ev.player.id, ev.msg)

    def onReady(self, ev):  
        self.notifyAllExcept(ev.player, 'ready', ev.player.id)


games = [Game(i) for i in xrange(10)]


class GameHandler(WSJsonRPCHandler):

    def init(self):
        self.player = None

    def connectionLost(self, reason):
        player = self.player
        if player is None:
            return

        player.game.leave(player)
        self.player = None

    # XXX lack of param check
    def do_join(self, nickname, gameID):
        if self.player:
            player = self.player
        else:
            player = Player(nickname, self)
        game = games[gameID]

        ok, msg = games[gameID].join(player)

        if not ok:
            return {'ok': ok, 'msg': msg}
        # set attr
        self.player = player
        return {'ok': ok, 'msg': msg, 'selfID': player.id, 
            'players': [ {'id': p.id, 'nickname': p.nickname, 'colorID': p.color, 
                'ready': p.ready} for p in game.players()]
        }

    def do_chat(self, msg):
        if self.player is None:
            return {'ok': False}

        player = self.player
        player.game.chat(player, msg)
        return {'ok': True}
    
    def  do_ready(self):
        if self.player is None:
            return {'ok': False}
        
        player = self.player
        player.game.ready(player)
        return {'ok': True}
