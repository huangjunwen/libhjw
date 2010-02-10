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


定义 tiepoint 位置编号:

                  0   1   2
                +---+---+---+
             11 |           | 3
                +           +
             10 |           | 4
                +           +
              9 |           | 5
                +---+---+---+
                  8   7   6

meeple 站位可以为任意 tiepoint 编号 (0~11) 以及站在修道院上 (12), 或者不占(-1)

"""

from random import shuffle

################
#     const    #
################

FIELD = 1

ROAD = 2

CITY = 3

CLOISTER = 4

################
#     class    #
################

class Board(object):

    def __init__(self):
        self.reset()

    def reset(self):
        """
        Reinit the board for a new game
        """
        self.coords = {}                                                        # {coord: tile}
        self.open_tps = {}                                                      # {tilepoint: terra}
        self.all_tps = {}                                                       # {tilepoint: terra}
        self.all_terra = set()                                                  # set([terra,])

        # shuffle the pile and add the first one
        self.tile_pile = TilePile.shuffle()
        self.top_tile_idx = 0
        self.put_top_tile((0, 0), is_first=True)

    @property
    def top_tile(self):
        """
        Get the top tile on the tile pile
        """
        try:
            return self.tile_pile[self.top_tile_idx]
        except IndexError:
            return None

    def put_top_tile(self, coord, rotation=0, is_first=False):
        """
        Put the top tile on the tile pile into the board
        """
        # some checks
        tile = self.top_tile
        if not tile:
            return False
        tile._set_pos(coord, rotation)

        if not self._check_tile_on_board(tile, is_first):
            return False

        # add tile itself
        self.coords[tile.coord] = tile
        self.top_tile_idx += 1

        # add terras on tile
        for terra in tile._pre_join_terra:
            if terra.closed:                                                    # if the terra is itself closed (CLOISTER)
                self.all_terra.add(terra)
                continue

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
            
        del tile._pre_join_terra
        tile.board = self

        return True

    def _check_tile_on_board(self, tile, is_first):
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
            if tile.bounds[i] != t.bounds[(i+2) % 4]:
                return False
        
        if not is_first and not has_neighbour:
            return False
        return True



class Terra(object):
    """
    Terra is generated by Tiles and joined in Board
    """
    
    ### !! the following methods should be called by Tile only

    def __init__(self, tile, terra_type, tps):
        self.terra_type = terra_type                                            # FIELD/ROAD/CITY/CLOISTER
        tps = set(tps)
        self.open_tps = tps                                                     # set of open tiepoints
        self.all_tps = tps                                                      # set of all tiepoints
        self.all_tiles = set([tile,])                                           # set of all tiles
        self.adjacent = set()                                                   # set of all adjacent terra
                                                                                #   tile should add adjacent after __init__
    ### !! the following methods should be called by Board only

    def _close_tiepoint(self, tp):
        if type(tp) is set:
            self.open_tps -= tp
        else:
            self.open_tps.discard(tp)

    def _join(self, terra):
        assert self.terra_type == terra.terra_type
        self.open_tps |= terra.open_tps
        self.all_tps |= terra.all_tps
        self.all_tiles |= terra.all_tiles
        self.adjacent |= terra.adjacent
        for t in terra.adjacent:
            t.adjacent.remove(terra)
            t.adjacent.add(self)

    ### attributes and properties are public

    @property
    def closed(self):
        return not self.open_tps



class TilePile(type):

    """
    想要放入 TilePile 的类需要继承 TileBase, 并且类名为 Tile

    Tile 子类指定以下类变量, 例如中间一条路, 两旁是农田的 Tile 子类可以这样表达:

        terra_proto = [(ROAD, [1,7]), (FIELD, [2,3,4,5,6]), (FIELD, [8,9,10,11,0])]

    TilePile 将之一份变 4 份 (每一个 rotation 一份), 同时生成一个类变量 bounds, 如上例子:

        bounds[0] = (ROAD, FIELD, ROAD, FIELD)
        bounds[1] = (FIELD, ROAD, FIELD, ROAD)
        ...

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

        # transform attributes
        terra_proto = attr['terra_proto']
        tp2tt = dict([(tp, tt) for tt, tps in terra_proto for tp in tps])       # {tiepoint: terra_type}
        bounds = [tp2tt[x] for x in (1, 4, 7, 10)]
        t, b = [], []
        for i in xrange(4):
            t.append(terra_proto)
            b.append(bounds)
            terra_proto = [(tt, map(lambda x: (x+3)%12, tps)) for tt, tps in terra_proto]
            bounds = bounds[-1:] + bounds[:-1]
        attr['bounds'] = b
        attr['terra_proto'] = t

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

    __metaclass__ = TilePile

    coord2tiepoint = {
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
    }

    def __init__(self, idx):
        self.idx = idx
        self.board = None

    def _set_pos(self, coord, rotation):
        cls = self.__class__
        self.coord = coord
        rotation %= 4
        self.bounds = cls.bounds[rotation]
        self.terra_proto = cls.terra_proto[rotation]
        self._make_pre_join_terra()

    def _make_pre_join_terra(self):
        # make terra instance on this tile
        pre_join_terra = []
        x, y = self.coord
        coord2tiepoint = self.coord2tiepoint
        for tt, tps in self.terra_proto:
            tps = map(lambda z: coord2tiepoint[z](x, y), tps)
            pre_join_terra.append(Terra(self, tt, tps))

        # link them
        for first, second in self.adjacent:
            first, second = pre_join_terra[first], pre_join_terra[second]
            first.adjacent.add(second)
            second.adjacent.add(first)
        self._pre_join_terra = pre_join_terra


################
#     tiles    #
################

class Tile(TileBase):
    spirit_coord = (0, 0)
    terra_proto = [(ROAD, [1,]), (FIELD, [2, 3]), (ROAD, [4,]), (FIELD, [5, 6]), (ROAD, [7,]), 
        (FIELD, [8, 9]),
        (ROAD, [10,]),
        (FIELD, [11, 0])]
    adjacent = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 7), (7, 0)]
    amount = 1


class Tile(TileBase):
    spirit_coord = (1, 0)
    terra_proto = [(CITY, range(12))]
    adjacent = []
    amount = 1


class Tile(TileBase):
    spirit_coord = (2, 0)
    terra_proto = [(FIELD, range(12)), (CLOISTER, [])]
    adjacent = [(0, 1)]
    amount = 4


class Tile(TileBase):
    spirit_coord = (3, 0)
    terra_proto = [(FIELD, [x for x in xrange(12) if x != 7]), (CLOISTER, []), (ROAD, [7,])]
    adjacent = [(0, 1), (1, 2), (2, 0)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (4, 0)
    terra_proto = [(FIELD, [11, 0, 1, 2, 3]), (ROAD, [4,]), (FIELD, [5, 6]), (ROAD, [7,]), 
        (FIELD, [8, 9]),
        (ROAD, [10,])]
    adjacent = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 0)]
    amount = 4
    start = True

