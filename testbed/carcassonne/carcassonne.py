# -*- encoding=utf-8 -*-

import re
from itertools import cycle
from twisted.python import log
from wsjson import *
from event import EventSrc
from board import Board, Meeple
import tiles

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

        # a notifier to broadcast events
        self.notifier = GameNotifier(self)

        # players dict
        self.id2player = {}
        self.nick2player = {}
        self.color2player = {}

        # game stuff
        self.ready_cnt = 0
        self.board = Board()
        self.curr_player = None
        self.player_loop = None

        self.cleanGame()

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

        if self.st == GAME_ST_GAMING:
            self.abortGame("因为下面的原因,　游戏不得不中止")

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
            player.ready = False
            self.ready_cnt -= 1
            assert self.ready_cnt >= 0
        self.fireEv('leave', player=player)

        # try to start game
        self.startGame()
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
        self.ready_cnt += 1
        self.fireEv('ready', player=player)
        self.startGame()
        return True

    def cleanGame(self):
        self.st = GAME_ST_IDLE 

        for p in self.players():
            p.ready = False
        self.ready_cnt = 0

        self.curr_player = None
        self.player_loop = None

        self.board.reset()
        self.fireEv('cleanGame')

    def abortGame(self, reason):
        self.cleanGame()
        self.fireEv('sysMsg', msg=reason)

    def startGame(self):    
        if self.st != GAME_ST_IDLE:
            return False

        if self.ready_cnt != len(self.id2player) or \
                self.ready_cnt < MIN_PLAYER_PER_GAME:
            return False

        self.st = GAME_ST_GAMING

        # create player_loop
        players = list(self.players())
        players.sort(lambda x, y: cmp(x.id, y.id))
        def create_player_loop():
            for p in cycle(players):
                self.curr_player = p
                yield p
        self.player_loop = create_player_loop()
        self.player_loop.next()

        self.fireEv('startGame', start_player=self.curr_player, 
            start_tile=self.board[0, 0])
        return True



class GameNotifier(object):
    
    def __init__(self, game):
        self.game = game
        for ev_name in ('join', 'leave', 'chat', 'sysMsg', 'ready', 'startGame',
            'cleanGame'):
            game.addEvListener(ev_name, getattr(self, 
                'on' + ev_name[0].upper() + ev_name[1:]))

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

    def onSysMsg(self, ev):   
        self.notifyAll('sysMsg', ev.msg)

    def onReady(self, ev):  
        self.notifyAllExcept(ev.player, 'ready', ev.player.id)

    def onStartGame(self, ev):
        self.notifyAll('startGame', ev.start_player.id, ev.start_tile.id, ev.start_tile.tile_idx)

    def onCleanGame(self, ev):
        self.notifyAll('cleanGame')
        

games = [Game(i) for i in xrange(10)]


class GameHandler(WSJsonRPCHandler):

    DEBUG = False

    def init(self):
        self._debug = False
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
        
        if self.DEBUG:
            not_handle = False
            if msg == '>>>':
                self._debug = True
                self.notify('chat', (self.player.id, "<debug mode on>"))
            elif msg == '<<<':
                self._debug = False
                self.notify('chat', (self.player.id, "<debug mode off>"))
            elif self._debug:
                try:
                    ret = eval(msg)
                except Exception, e:
                    ret = e
                self.notify('chat', (self.player.id, repr(ret)))
            else:
                not_handle = True
            
            if not not_handle:
                return {'ok': True}
            
        player = self.player
        player.game.chat(player, msg)
        return {'ok': True}
    
    def  do_ready(self):
        if self.player is None:
            return {'ok': False}
        
        player = self.player
        player.game.ready(player)
        return {'ok': True}
