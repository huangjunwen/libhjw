# -*- encoding=utf-8 -*-

import re
from twisted.python import log
from wsjson import *

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

    def notify(self, method, *params):
        self.handler.notify(method, params)


GAME_ST_IDLE = 0

GAME_ST_GAMING = 1

MAX_PLAYER_PER_GAME = 5


class Game(object):

    def __init__(self, id):
        self.id = id
        self.st = GAME_ST_IDLE 

        # dicts
        self.id2player = {}
        self.nick2player = {}
        self.color2player = {}

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

        self.id2player[player.id] = player
        self.nick2player[player.nickname] = player
        for color in xrange(MAX_PLAYER_PER_GAME):
            if color not in self.color2player:
                break
        self.color2player[color] = player

        player.game = self
        player.color = color
        return True, ""

    def leave(self, player):
        if player.id not in self.id2player:
            return False
        
        p = self.id2player.pop(player.id)
        assert p == player
        p = self.nick2player.pop(player.nickname)
        assert p == player
        p = self.color2player.pop(player.color)
        assert p == player

        player.room = None
        player.color = None
        return True


games = [Game(i) for i in xrange(10)]


class GameHandler(WSJsonRPCHandler):

    def init(self):
        self.player = None

    def connectionLost(self, reason):
        player = self.player
        if player is None:
            return

        player.game.leave(player)
        for p in player.game.players():
            p.notify('leave', player.id)
        self.player = None

    # XXX lack of param check
    def do_join(self, nickname, gameID):
        if self.player is not None:
            self.close()
            return

        player = Player(nickname, self)
        game = games[gameID]
        ok, msg = games[gameID].join(player)
        if not ok:
            return {'ok': ok, 'msg': msg}
        
        # set attr
        self.player = player

        # broadcast and return
        for p in game.players():
            if p != player:
                p.notify('join', player.id, player.nickname, player.color)
        return {'ok': ok, 'msg': msg, 'selfID': player.id, 
            'players': [ {'id': p.id, 'nickname': p.nickname, 'colorID': p.color, 
                'ready': p.ready} for p in game.players()]
        }

    def do_chat(self, msg):
        if not hasattr(self, 'player'):
            return {'ok': False}

        # broadcast and return
        player = self.player
        for p in player.game.players():
            if p != player:
                p.notify('chat', player.id, msg)
        return {'ok': True}
