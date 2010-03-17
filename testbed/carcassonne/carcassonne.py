# -*- encoding=utf-8 -*-

import re
from itertools import cycle, chain
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

MEEPLE_PER_PLAYER = 7


class Game(EventSrc):

    def __init__(self, id):
        super(Game, self).__init__()
        self.id = id
        self.st = GAME_ST_IDLE 

        # players dict
        self.id2player = {}
        self.nick2player = {}
        self.color2player = {}

        # game stuff
        self.board = Board()
        self.completedTerra = set()
        self.board.addEvListener('terraComplete', 
            lambda ev: self.completedTerra.add(ev.terra))

        # before game
        self.ready_cnt = 0
        self.cleanGame()

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

        self.board.reset()
        self.meeples = dict([(color, [Meeple(color) for i in xrange(MEEPLE_PER_PLAYER)]) 
            for color in xrange(MAX_PLAYER_PER_GAME)])

        self.scores = [0 for i in xrange(MAX_PLAYER_PER_GAME)]
        self.curr_player = None
        self.curr_tile = None
        self.curr_meeple = None
        self.player_loop = None

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
                self.curr_meeple = None
                for m in self.meeples[p.color]:
                    if not m.used:
                        self.curr_meeple = m
                        break
                yield
        self.player_loop = create_player_loop()
        self.player_loop.next()
        self.curr_tile = self.board[0, 0]

        self.fireEv('startGame', start_player=self.curr_player, 
            start_tile=self.curr_tile, 
            remain=len(self.board.tile_pile))
        return True

    def putMeeple(self, player, terra_idx, pos):
        if self.st != GAME_ST_GAMING or player != self.curr_player or \
                self.curr_meeple is None:
            return False

        meeple = self.curr_meeple
        tile = self.curr_tile

        if not tile.onBoard:
            return False

        if not self.board.canPut(tile):
            return False

        if terra_idx not in self.board.unoccupiedTerraIdx(tile):
            return False
        
        self.curr_meeple.put(tile, terra_idx, pos)
        self.fireEv('putMeeple', player=player, meeple=meeple, tile=tile, 
            terra_idx=terra_idx, pos=pos)
        return True

    def pickMeeple(self, player):
        if self.st != GAME_ST_GAMING or player != self.curr_player or \
                self.curr_meeple is None:
            return False
        
        if not self.curr_meeple.used:
            return True
        
        self.curr_meeple.pick()
        self.fireEv('pickMeeple', meeple=self.curr_meeple)
        return True

    def turnEnd(self, player):
        if self.st != GAME_ST_GAMING or player != self.curr_player:
            return False

        # not yet put on board
        if not self.curr_tile.onBoard:
            return False

        # put tile
        tile = self.curr_tile
        if self.board[tile.coord] is not tile:
            if not self.board.putTile(tile):
                return False
        
        # put meeple
        meeple = self.curr_meeple
        if meeple and meeple.used:
            terra = self.board.terraOnTileByIdx(tile, meeple.terra_idx)
            res = terra.putMeeple(meeple)
            assert res

        # ok, now we calculate the scores
        self.calcScore()

        # take turn
        self.player_loop.next()
        self.curr_tile = self.board.pickTile()
        self.fireEv('takeTurn', player=self.curr_player)
        return True

    def calcScore(self, last=False):
        for terra in self.completedTerra:
            # XXX pickMeeple and add score
            pass
        self.completedTerra = set()

    def putTile(self, player, x, y):
        if self.st != GAME_ST_GAMING or player != self.curr_player:
            return False
        
        tile = self.curr_tile
        coord = (x, y)
        if self.board[coord] is not None:
            return False
        
        tile.setCoord(coord)
        self.fireEv('putTile', tile=tile, coord=coord)
        return True

    def moveTile(self, player, x, y):
        if self.st != GAME_ST_GAMING or player != self.curr_player:
            return False
        
        tile = self.curr_tile
        coord = (x, y)
        if self.board[coord] is not None:
            return False
        
        tile.setCoord(coord)
        self.fireEv('moveTile', player=player, coord=coord)
        return True

    def rotateTile(self, player, rotation):
        if self.st != GAME_ST_GAMING or player != self.curr_player:
            return False
        
        tile = self.curr_tile
        if not tile.onBoard:
            return False

        tile.rotate(rotation)
        self.fireEv('rotateTile', player=player, rotation=tile.rotation)
        return True
        

class GameNotifier(object):
    
    def __init__(self, game):
        self.game = game
        for ev_name in ('join', 'leave', 'chat', 'sysMsg', 'ready', 'startGame',
            'cleanGame',
            'pickMeeple',
            'putMeeple',
            'putTile',
            'moveTile',
            'rotateTile',
            'takeTurn'):
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
        self.notifyAll('startGame', ev.start_player.id, ev.start_tile.id, 
            ev.start_tile.tile_idx,
            ev.remain)

    def onCleanGame(self, ev):
        self.notifyAll('cleanGame')

    def onPutMeeple(self, ev):
        self.notifyAll('putMeeple', ev.meeple.id, ev.meeple.color, ev.tile.id, ev.pos)
        
    def onPickMeeple(self, ev):
        if ev.score:
            self.notifyAll('pickMeeple', ev.meeple.id, ev.score)
        else:
            self.notifyAll('pickMeeple', ev.meeple.id)
    
    def onPutTile(self, ev):
        self.notifyAll('putTile', ev.tile.id, ev.tile.tile_idx, 
            {'gX': ev.coord[0], 'gY': ev.coord[1]})

    def onMoveTile(self, ev):
        self.notifyAllExcept(ev.player, 'moveTile', 
            {'gX': ev.coord[0], 'gY': ev.coord[1]})

    def onRotateTile(self, ev):
        self.notifyAllExcept(ev.player, 'rotateTile', ev.rotation)

    def onTakeTurn(self, ev):   
        self.notifyAll('takeTurn', ev.player.id)


games = [Game(i) for i in xrange(10)]


class GameHandler(WSJsonRPCHandler):

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
                'ready': p.ready} for p in game.players()],
            'meeples': [ {'id': m.id, 'colorID': m.color} 
                for m in chain(*(x for x in game.meeples.itervalues())) ]
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
    
    def do_ready(self):
        if self.player is None:
            return {'ok': False}
        
        player = self.player
        player.game.ready(player)
        return {'ok': True}

    def do_putMeeple(self, terra_idx, pos):
        if self.player is None:
            return {'ok': False}
        
        player = self.player
        if not player.game.putMeeple(player, terra_idx, pos):
            return {'ok': False}
        return {'ok': True}

    def do_pickMeeple(self):
        if self.player is None:
            return {'ok': False}

        player = self.player
        if not player.game.pickMeeple(player):
            return {'ok': False}
        return {'ok': True}

    def do_turnEnd(self):
        if self.player is None:
            return {'ok': False}

        player = self.player
        if not player.game.turnEnd(player):
            return {'ok': False}
        return {'ok': True}

    def do_putTile(self, x, y):
        if self.player is None:
            return {'ok': False}

        player = self.player
        if not player.game.putTile(player, x, y):
            return {'ok': False}
        return {'ok': True}

    def do_moveTile(self, x, y):
        if self.player is None:
            return {'ok': False}

        player = self.player
        if not player.game.moveTile(player, x, y):
            return {'ok': False}
        return {'ok': True}

    def do_rotateTile(self, rotation):
        if self.player is None:
            return {'ok': False}

        player = self.player
        if not player.game.rotateTile(player, rotation):
            return {'ok': False}
        return {'ok': True}
