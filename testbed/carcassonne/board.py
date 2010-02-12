#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

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

from random import shuffle


class Meeple(object):

    def __init__(self, player):
        self.player = player



class Board(object):
    """
    Game board

    Public methods and attributes:

        last_tile (r): last picked up tile
        tiles_on_board (r): set of tiles on board
        tiles_discard (r): set of tiles discard by players
        tiles_on_pile (r): list of tiles on pile. tiles_on_pile[0] is the next pile to be picked up

        open_tps (r): dict of open tiepoint -> terra
        all_tps (r): dict of tiepoint -> terra
        all_terra (r): all terra on the board

        reset (x): reset the board
        pick_tile (x): pick up a tile from pile
        put_tile (x): put the top tile on pile to the board
        discard_tile (x): discard a tile
        put_meeple (x): put a meeple on the tile
        __getitem__ (x): get a tile on the specified coord

    """

    def __init__(self):
        self.reset()

    def reset(self):
        """
        Reinit the board for a new game
        """
        self.open_tps = {}                                                      # {tilepoint: terra}
        self.all_tps = {}                                                       # {tilepoint: terra}
        self.all_terra = set()                                                  # set([terra,])

        self.last_tile = None
        self.coords = {}                                                        # {coord: tile}
        self.tiles_on_pile = TilePile.shuffle()
        self.tiles_on_board = set()
        self.tiles_discard = set()

        first = self.pick_tile()
        self.put_tile(first, 0, 0)

    def __getitem__(self, coord):
        """
        Get the tile on coord
        """
        try:
            return self.coords[coord]
        except KeyError:
            raise IndexError("No tile found at %r" % (coord,))

    def pick_tile(self):
        """
        Pick up a tile from pile
        """
        last_tile = self.last_tile
        # make sure the last one has been handled
        assert last_tile is None or last_tile in tiles_on_board or \
            last_tile in tiles_discard
        try:
            self.last_tile = self.tiles_on_pile.pop(0)
        except IndexError:
            self.last_tile = None
        return self.last_tile

    def discard_tile(self, tile):
        assert tile is self.last_tile and tile not in self.tiles_on_board
        self.tiles_discard.add(tile)

    def put_tile(self, tile, x, y, rotation=0, is_first=False):
        """
        Put the top tile on the tile pile into the board
        """
        # check
        assert tile is self.last_tile and tile not in self.tiles_discard
        if tile.board:
            return True

        coord = (x, y)
        tile._set_pos(coord, rotation)
        if not self._check_tile_pos(tile, not self.tiles_on_board):
            return False

        # add tile
        self.coords[tile.coord] = tile
        self.tiles_on_board.add(tile)

        # add terras on tile
        for terra in tile._make_pre_join_terra():
            joint_terra = set()
            closed_tps = set()

            # get the common open tiepoints
            for tp in terra.open_tps:
                if tp not in self.open_tps:
                    continue
                # remove from board's open_tps set and the t's open_tps set
                t = self.open_tps.pop(tp)
                t._close_tiepoint(tp)
                # collect
                closed_tps.add(tp)
                joint_terra.add(t)
            # remove closed_tps in terra
            terra._close_tiepoint(closed_tps)
                
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
            
        tile.board = self
        return True

    def _check_tile_pos(self, tile, is_first):
        if tile.coord in self.coords:                                          # occupied
            return False

        # check 4 neighbours and their boundaries
        has_neighbour = False
        x, y = tile.coord
        neighbour_coords = ((x, y+1), (x+1, y), (x, y-1), (x-1, y))
        for i in xrange(4):                                                     
            c = neighbour_coords[i]
            if c not in self.coords:
                continue
            has_neighbour = True
            t = self.coords[c]
            if tile.bounds[i] != t.bounds[(i+2) % 4]:                           # check bounds
                return False
        
        if not is_first and not has_neighbour:
            return False
        return True

    def put_meeple(self, tile, meeple, tpn):
        assert tile is self.last_tile and tile in self.tiles_on_board
        terra = tile.terra(tpn)
        if not terra:
            return False
        if terra.meeples:
            return False
        terra.meeples.add(meeple)
        return True



class TilePile(type):

    """
    想要放入 TilePile 的类需要继承 TileBase, 并且类名为 Tile

    Tile 子类指定以下类变量, 例如中间一条路, 两旁是农田的 Tile 子类可以这样表达:

        terra_proto = [(ROAD, [1,7], None), (FIELD, [2,3,4,5,6], None), 
            (FIELD, [8,9,10,11,0], None)]

    TilePile 根据此变量生成两个个类变量, 如上例子:

        bounds = [ROAD, FIELD, ROAD, FIELD]

    另外 Tile 子类必须指定以下这些类变量

        有多少张这种 Tile:
            amount = 3 
    
        terra 的邻接关系:
            adjacent = [(0, 1), (0, 2)]

        图片上这种 tile 的坐标:
            spirit_coord = (x, y)

        是否是第一块 Tile, 默认为 False:
            start = True
    """

    tile_cls = []

    start_tile_cls = None

    spirit_coords = set()

    def __new__(mcls, name, base, attr):
        if name != 'Tile':
            return type.__new__(mcls, name, base, attr)

        # uniq coord
        spirit_coord = attr['spirit_coord']
        assert spirit_coord not in TilePile.spirit_coords
        name += '_%d_%d' % spirit_coord
        TilePile.spirit_coords.add(spirit_coord)

        # add attributes
        tpn2tcls = [None] * 13
        for tcls, tpns, _ in attr['terra_proto']:
            for tpn in tpns:
                tpn2tcls[tpn] = tcls
        attr['bounds'] = [tpn2tcls[x] for x in (1, 4, 7, 10)]

        # make and put to the pile
        ret = type.__new__(mcls, name, base, attr)
        amount = attr['amount']
        if attr.get('start', False):
            assert TilePile.start_tile_cls is None
            amount -= 1
            TilePile.start_tile_cls = ret
        TilePile.tile_cls.extend([ret] * amount)
        return ret

    @staticmethod
    def shuffle():
        """
        Shuffle the tile pile
        Return shuffled tile pile (Tile instances)
        """
        shuffle(TilePile.tile_cls)

        ret = [TilePile.start_tile_cls(0)]
        for i in xrange(len(TilePile.tile_cls)):
            ret.append(TilePile.tile_cls[i](i+1))
        return ret



class TileBase(object):
    """
    Tile contains terra and terra can join together

    Public methods and attributes:

        bounds (r): 4 bound of this tile
        terra_proto (r): meta information of terra
        board (r): None or the board on which this tile is settled

        the following should be called only after this tile has put on board

        tiepoint (x): tiepoint number -> tiepoint
        terra (x): tiepoint number -> terra
    """

    __metaclass__ = TilePile

    tpn2tp = {
        0: lambda x, y: (0, 3*x-1, 2*y+1),
        1: lambda x, y: (0, 3*x, 2*y+1),
        2: lambda x, y: (0, 3*x+1, 2*y+1),
        3: lambda x, y: (1, 2*x+1, 3*y+1),
        4: lambda x, y: (1, 2*x+1, 3*y),
        5: lambda x, y: (1, 2*x+1, 3*y-1),
        6: lambda x, y: (0, 3*x+1, 2*y-1),
        7: lambda x, y: (0, 3*x, 2*y-1),
        8: lambda x, y: (0, 3*x-1, 2*y-1),
        9: lambda x, y: (1, 2*x-1, 3*y-1),
        10: lambda x, y: (1, 2*x-1, 3*y),
        11: lambda x, y: (1, 2*x-1, 3*y+1),
        12: lambda x, y: (2, x, y),
    }

    def __init__(self, idx):
        self.idx = idx
        self.board = None

    def __repr__(self):
        ret = "<%s" % self.__class__.__name__
        if hasattr(self, 'coord'):
            ret += " at %r" % (self.coord,)
        return ret + ">"

    def _rotate(self, rotation):
        rotation %= 4
        if not rotation:
            return

        cls = self.__class__
        r = 4 - rotation
        self.bounds = cls.bounds[r:] + cls.bounds[:r]

        r = rotation * 3
        def rotate_tpns(tpns):
            return [tpn == 12 and 12 or (tpn+r)%12 for tpn in tpns]
        self.terra_proto = [(tcls, rotate_tpns(tpns), ex) 
            for tcls, tpns, ex in cls.terra_proto]
        
    def _set_pos(self, coord, rotation):
        self.coord = coord
        self._rotate(rotation)
        x, y = coord
        tpn2tp = self.tpn2tp
        self.tiepoint = lambda tpn: tpn2tp[tpn](x, y)

    def _make_pre_join_terra(self):
        # make terra instances on this tile
        pre_join_terra = []
        for tcls, tpns, ex in self.terra_proto:
            tps = map(self.tiepoint, tpns)
            pre_join_terra.append(tcls(self, tps, ex))

        # add adjacent link
        for first, second in self.adjacent:
            first, second = pre_join_terra[first], pre_join_terra[second]
            first.adjacent.add(second)
            second.adjacent.add(first)
        return pre_join_terra

    def terra(self, tpn):
        """
        Get the terra at tpn
        """
        if self.board is None:
            raise RuntimeError("This tile has not yet placed on board")
        return self.board.all_tps.get(self.tiepoint(tpn), None)



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

    def _close_tiepoint(self, tp):
        if type(tp) is set:
            self.open_tps -= tp
        else:
            self.open_tps.discard(tp)

    def _join(self, terra):
        assert type(self) is type(terra)
        self.open_tps |= terra.open_tps
        self.all_tps |= terra.all_tps
        self.all_tiles |= terra.all_tiles
        self.adjacent |= terra.adjacent
        self.meeples |= terra.meeples
        for t in terra.adjacent:
            t.adjacent.remove(terra)
            t.adjacent.add(self)

    ### attributes and properties are public

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
