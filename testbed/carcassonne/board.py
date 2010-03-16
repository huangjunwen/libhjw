#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8:nu:nowrap

"""
= design =

== tile ==

即游戏中的小块, 可以旋转后放入地图中, 必须连接到已经存在的其它 tile 边上

游戏初始化的时候摆下的第一个 tile 的坐标是 (0, 0), 它的左右两边的坐标分别是 (-1, 0), (1, 0), 它的上下两块的坐标分别是 (0, 1), (0, -1)

定义 tile 旋转
 * rotation % 4 == 0 -> 无旋转
 * rotation % 4 == 1 -> 顺时针 90 度 
 * rotation % 4 == 2 -> 顺时针 180 度 
 * rotation % 4 == 3 -> 顺时针 270 度


定义 tile 边 (boundary) 编号

                      0
                +---+---+---+
                |           |  
                +           +
              3 |           | 1
                +           +
                |           |  
                +---+---+---+
                      2
    
== terra ==

地形有数种, 包括 ROAD/FIELD/CITY/CLOISTER, 扩展包可能还包括河流等, 每一个 tile 上包含一种到数种地形

terra 有可能闭合或尚未闭合, 通过 tiepoint (每个 tile 每一边共有三个 tiepoint) 接合, 可以查询与之相邻的其它 terra (例如农田旁的城市)

定义 tiepoint 为以三元组, 跟 tile 坐标的关系如下图:

                                           y
                                            
                                          /^\
                                           |
                                           |
                 
                 (0,-1,1) (0,0,1)  (0,1,1)  (0,2,1)  (0,3,1)  (0,4,1)
                +--------+--------+--------+--------+--------+--------+
                |                          |                          |
    (1, -1, 1)  |                       (1,1,1)                       | (1, 3, 1)
                |                          |                          |
                +                          +                          +
                |                          |                          |
    (1, -1, 0)  |          (0, 0)       (1,1,0)       (1, 0)          | (1, 3, 0)           -----> x
                |                          |                          |
                +                          +                          +
                |                          |                          |
    (1, -1, -1) |                       (1,1,-1)                      | (1, 3, -1)
                |                          |                          |
                +--------+--------+--------+--------+--------+--------+
                (0,-1,-1) (0,0,-1) (0,1,-1) (0,2,-1) (0,3,-1) (0,4,-1)

更一般地

                 (0,3x-1,2y+1) (0,3x,2y+1) (0,3x+1,2y+1)
                +-------------+-----------+-------------+
                |                                       |
 (1,2x-1,3y+1)  |                                       | (1,2x+1,3y+1)
                |                                       |
                +                                       +
                |                                       |
   (1,2x-1,3y)  |                (x, y)                 | (1,2x+1,3y)
                |                                       |
                +                                       +
                |                                       |
 (1,2x-1,3y-1)  |                                       | (1,2x+1,3y-1)
                |                                       |
                +-------------+-----------+-------------+
                 (0,3x-1,2y-1) (0,3x,2y-1) (0,3x+1,2y-1)


另外, 坐标为 (x, y) 的 Tile 上定义特殊 tiepoint 坐标 (2, x, y), 用于表示修道院, 这类型的 tiepoint 总是闭合的

定义 tiepoint 位置编号 tpn (tiepoint number):

                  0   1   2
                +---+---+---+
             11 |           | 3
                +           +
             10 |    12     | 4
                +           +
              9 |           | 5
                +---+---+---+
                  8   7   6

"""

from random import randint
from event import EventSrc


class Meeple(object):

    create_cnt = 0

    def __init__(self, color):
        self.id = "M%d" % Meeple.create_cnt
        Meeple.create_cnt += 1
        self.color = color
        self.pick()
    
    def put(self, tile, terra_idx, pos):
        self.tile = tile
        self.terra_idx = terra_idx
        self.pos = pos

    def pick(self):
        self.tile = None
        self.terra_idx = None
        self.pos = None                               # XXX 

    @property
    def used(self):
        return self.tile is not None
    


tpn2tp = [
    lambda x, y: (0, 3*x-1, 2*y+1),
    lambda x, y: (0, 3*x, 2*y+1),
    lambda x, y: (0, 3*x+1, 2*y+1),
    lambda x, y: (1, 2*x+1, 3*y+1),
    lambda x, y: (1, 2*x+1, 3*y),
    lambda x, y: (1, 2*x+1, 3*y-1),
    lambda x, y: (0, 3*x+1, 2*y-1),
    lambda x, y: (0, 3*x, 2*y-1),
    lambda x, y: (0, 3*x-1, 2*y-1),
    lambda x, y: (1, 2*x-1, 3*y-1),
    lambda x, y: (1, 2*x-1, 3*y),
    lambda x, y: (1, 2*x-1, 3*y+1),
    lambda x, y: (2, x, y),
]


class Board(EventSrc):
    """
    Game board

    Attributes:

        last_tile (r): last picked up tile
        tile_pile (r): tiles remain on the pile
        tiles_on_board (r): set of tiles on board
        tiles_discard (r): set of tiles discard by players

        open_tps (r): dict of open tiepoint -> terra
        all_tps (r): dict of tiepoint -> terra
        all_terra (r): all terra on the board

    Events:
        terraComplete
    """

    def __init__(self):
        super(Board, self).__init__()
        self.reset()

    def reset(self):
        """
        Reinit the board for a new game
        """
        self.open_tps = {}                                                      # {tilepoint: terra}
        self.all_tps = {}                                                       # {tilepoint: terra}
        self.all_terra = set()                                                  # set([terra,])

        self.last_tile = None
        self.last_tile_handled = True
        self.coords = {}                                                        # {coord: tile}
        self.tile_pile = TilePile()
        self.tiles_on_board = set()
        self.tiles_discard = set()

        first = self.pickTile()
        first.setCoord((0, 0))
        self.putTile(first)

    def pickTile(self):
        """
        Pick up a tile from pile
        """
        assert self.last_tile_handled
        self.last_tile = self.tile_pile.pick()
        if self.last_tile:
            self.last_tile_handled = False
        return self.last_tile

    def discardTile(self, tile):
        """
        Discard a tile
        """
        assert not self.last_tile_handled and tile is self.last_tile
        self.tiles_discard.add(tile)
        self.last_tile_handled = True

    def putTile(self, tile):
        """
        Put the top tile on the tile pile into the board
        """
        assert not self.last_tile_handled and tile is self.last_tile

        # test coord
        if not self.canPut(tile):
            return False

        # add tile to board
        self.coords[tile.coord] = tile
        self.tiles_on_board.add(tile)

        # add terras on tile
        for terra in tile._makePreJoinTerra():
            joint_terra = set()
            closed_tps = set()

            # get the common open tiepoints
            for tp in terra.open_tps:
                if tp not in self.open_tps:
                    continue
                # remove from board's open_tps set and the t's open_tps set
                t = self.open_tps.pop(tp)
                t._closeTiepoint(tp)
                # collect
                closed_tps.add(tp)
                joint_terra.add(t)
            # remove closed_tps in terra
            terra._closeTiepoint(closed_tps)
                
            # make a big one and place to all_terra
            for t in joint_terra:
                terra._join(t)
                self.all_terra.remove(t)
            self.all_terra.add(terra)
            
            # update board's tps dict
            for tp in terra.open_tps:
                self.open_tps[tp] = terra
            for tp in terra.all_tps:
                self.all_tps[tp] = terra
        self.last_tile_handled = True

        # fire events
        cloister = None
        for terra in self.terraOnTile(tile):
            if not terra.closed:
                continue
            if isinstance(terra, CLOISTER):
                cloister = terra
                continue
            self.fireEv('terraComplete', terra=terra)
    
        neighbour_cnt = 0
        for n in self.neighbours(tile):
            if n is None:
                continue
            neighbour_cnt += 1
            n.neighbour_cnt += 1
            if n.neighbour_cnt >= 8:
                terra = self.terraOnTileByIdx(n, -1)                # XXX CLOISTER always the last one
                if isinstance(terra, CLOISTER):
                    self.fireEv('terraComplete', terra=terra)
        tile.neighbour_cnt = neighbour_cnt
        if neighbour_cnt >= 8 and cloister:
            self.fireEv('terraComplete', terra=cloister)

        return True

    def canPut(self, tile):
        """
        Can the tile be put
        """
        if tile.coord in self.coords:
            if tile is self[tile.coord]:
                return True
            return False

        x, y = tile.coord

        # check 4 neighbours and their boundaries
        has_neighbour = False
        neighbour_coords = ((x, y+1), (x+1, y), (x, y-1), (x-1, y))
        neighbour_bounds = (2, 3, 0, 1)

        for i in xrange(4):                                                     
            c = neighbour_coords[i]
            if c not in self.coords:
                continue
            has_neighbour = True
            t = self.coords[c]
            if tile.bounds[i] != t.bounds[neighbour_bounds[i]]:                 # check bounds
                return False
        
        first = not self.tiles_on_board
        if not first and not has_neighbour:
            return False
        return True

    def unoccupiedTerraIdx(self, tile):
        """
        Get unoccupied terra indexes
        """
        ret = []
        for i in xrange(len(tile.terra_proto)):
            _, tpns, _ = tile.terra_proto[i]
            occupied = False
            for tpn in tpns:
                terra = self.open_tps.get(tile.tiepoint(tpn), None)
                if not terra:
                    continue

                if terra.meeples:
                    occupied = True
                    break
            
            if not occupied:
                ret.append(i)
        return ret

    def terraOnTileByIdx(self, tile, terra_idx):
        if tile not in self.tiles_on_board:
            return None
        
        _, tpns, _ = tile.terra_proto[terra_idx]
        return self.all_tps[tile.tiepoint(tpns[0])]

    def terraOnTile(self, tile): 
        return [self.terraOnTileByIdx(tile, i) for i in xrange(len(tile.terra_proto))]

    def neighbours(self, tile):
        assert tile in self.tiles_on_board
        x, y = tile.coord
        yield self[x - 1, y + 1]
        yield self[x, y + 1]
        yield self[x + 1, y + 1]
        yield self[x + 1, y]
        yield self[x + 1, y - 1]
        yield self[x, y - 1]
        yield self[x - 1, y - 1]
        yield self[x - 1, y]

    def __getitem__(self, coord):
        """
        Get the tile on coord
        """
        return self.coords.get(coord, None)


class TilePile(object):

    def __init__(self):
        self.tiles = [t() for t in TileMeta.tile_classes]
        self.id = 0
    
    def pick(self):
        if self.id == 0:
            ret = TileMeta.start_tile_cls()
        elif self.tiles:
            ret = self.tiles.pop(randint(0, len(self.tiles) - 1))
        else:
            return None
        ret.id = "T%d" % self.id
        self.id += 1
        return ret

    def __len__(self):
        if self.id == 0:
            return len(self.tiles) + 1
        return len(self.tiles)


class TileMeta(type):

    """
    想要放入 TileMeta 的类需要继承 TileBase, 并且类名为 Tile

    Tile 子类指定以下类变量, 例如中间一条路, 两旁是农田的 Tile 子类可以这样表达:

        terra_proto = [(FIELD, [8,9,10,11,0], None), (ROAD, [1,7], None), 
            (FIELD, [2,3,4,5,6], None)]

    (terra_proto 必须按照 tpn 从小到大排序)

    TileMeta 根据此变量生成一个类变量, 如上例子:

        bounds = [ROAD, FIELD, ROAD, FIELD]

    另外 Tile 子类必须指定以下这些类变量

        有多少张这种 Tile:
            amount = 3 
    
        terra 的邻接关系:
            adjacent = [(0, 1), (0, 2)]

        Tile 编号
            tile_idx = 11

        是否是第一块 Tile, 默认为 False:
            start = True
    """

    tile_classes = []

    start_tile_cls = None

    tile_idxes = set()

    def __new__(mcls, name, base, attr):
        if name != 'Tile':
            return type.__new__(mcls, name, base, attr)

        # uniq idx
        tile_idx = attr['tile_idx']
        assert tile_idx not in TileMeta.tile_idxes
        name += '_%d' % tile_idx
        TileMeta.tile_idxes.add(tile_idx)

        # add attributes
        t = attr['terra_proto']
        assert TileMeta._checkTerraOrder(t)
        tpn2tcls = [None] * 13
        for tcls, tpns, _ in t:
            for tpn in tpns:
                tpn2tcls[tpn] = tcls
        b = [tpn2tcls[x] for x in (1, 4, 7, 10)]
        
        # four rotation copy
        rotate_tpns = lambda tpns, r: [tpn == 12 and 12 or (tpn+3*r)%12 for tpn in tpns]
        terra_proto, bounds = [], []
        for r in xrange(4):
            terra_proto.append([(tcls, rotate_tpns(tpns, r), ex) for tcls, tpns, ex in t])
            bounds.append(b[-r:] + b[:-r])
        attr['terra_proto'] = terra_proto
        attr['bounds'] = bounds

        # make it
        ret = type.__new__(mcls, name, base, attr)
        amount = attr['amount']
        if attr.get('start', False):
            assert TileMeta.start_tile_cls is None
            amount -= 1
            TileMeta.start_tile_cls = ret
        TileMeta.tile_classes.extend([ret] * amount)
        return ret

    @staticmethod
    def _checkTerraOrder(terra_proto):
        min_tpn = -1
        for _, tpns, _ in terra_proto:
            new_min_tpn = min(tpns)
            if min_tpn > new_min_tpn:
                return False
            min_tpn = new_min_tpn
        return True


class TileBase(object):
    """
    Tile contains terra and terra can join together

    attributes:

        bounds (r): 4 bound of this tile
        terra_proto (r): meta information of terra

    """

    __metaclass__ = TileMeta

    def __init__(self):
        self.coord = (0, 0)
        self.rotation = 0
        self.bounds = None
        self.terra_proto = None
        self.neighbour_cnt = 0
        self.rotate(0)

    def __repr__(self):
        ret = "<%s(%d)" % (self.__class__.__name__, self.rotation)
        if self.coord:
            ret += " at %r" % (self.coord,)
        return ret + ">"

    def tiepoint(self, tpn):
        x, y = self.coord
        return tpn2tp[tpn](x, y)

    def setCoord(self, coord):
        """
        Set the coord
        """
        self.coord = coord

    def rotate(self, rotation=1):
        """
        Rotate the tile
        """
        cls = self.__class__
        r = self.rotation = (self.rotation + rotation) & 3                  # % 4
        self.bounds = cls.bounds[r]
        self.terra_proto = cls.terra_proto[r]

    def _makePreJoinTerra(self):
        # make terra instances on this tile
        pre_join_terra = []
        for tcls, tpns, ex in self.terra_proto:
            tps = [self.tiepoint(tpn) for tpn in tpns]
            pre_join_terra.append(tcls(self, tps, ex))

        # add adjacent link
        for first, second in self.adjacent:
            first, second = pre_join_terra[first], pre_join_terra[second]
            first.adjacent.add(second)
            second.adjacent.add(first)
        return pre_join_terra



class TerraMeta(type):
    
    def __repr__(cls):
        return cls.__name__


class Terra(object):
    """
    Terra is generated by Tiles and joined in Board

    Public methods and attributes:
        
        open_tps (r): set of open tiepoints
        all_tps (r): set of all tiepoints
        all_tiles (r): set of all tiles this terra covers
        extra (r): dict of extra attributes for this terra
        adjacent (r): set of adjacent terras
        closed (r): is this terra closed
        meeples (r): set of meeples on this terra
    """

    __metaclass__ = TerraMeta

    ### !! the following methods should be called by Tile only

    def __init__(self, tile, tps, extra):
        self.open_tps = set(tps)                                                # set of open tiepoints
        self.all_tps = set(tps)                                                 # set of all tiepoints
        self.all_tiles = set([tile,])                                           # set of all tiles
        self.extra = extra
        self.adjacent = set()                                                   # set of all adjacent terra
                                                                                #   tile should add adjacent after __init__
        self.meeples = set()

    ### !! the following methods should be called by Board only

    def _closeTiepoint(self, tp):
        if type(tp) is set:
            self.open_tps -= tp
        else:
            self.open_tps.discard(tp)

    def _join(self, terra):
        self.open_tps |= terra.open_tps
        self.all_tps |= terra.all_tps
        self.all_tiles |= terra.all_tiles
        self.adjacent |= terra.adjacent
        self.meeples |= terra.meeples
        for t in terra.adjacent:
            t.adjacent.remove(terra)
            t.adjacent.add(self)

    ### attributes and properties are public

    def putMeeple(self, meeple):
        """
        Put a meeple on this terra
        """
        if self.meeples:
            return False
        self.meeples.add(meeple)
        return True

    def pickMeeples(self):  
        ret = self.meeples
        for m in ret:
            m.pick()
        self.meeples = set()
        return ret

    @property
    def closed(self):
        return not self.open_tps

    def __repr__(self):
        return "<%s:%d:%s>" % (self.__class__.__name__,
            len(self.all_tiles),
            self.closed and "closed" or "open")


class FIELD(Terra):
    terra_type = 1


class ROAD(Terra): 
    terra_type = 2


class CITY(Terra):
    terra_type = 3


class CLOISTER(Terra):
    terra_type = 4
    def __init__(self, *args):
        Terra.__init__(self, *args)
        self.open_tps.clear()


# class RIVER(Terra):
#     terra_type = 5   
