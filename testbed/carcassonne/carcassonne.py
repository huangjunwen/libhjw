#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

"""
= design =

== tile ==

即游戏中的小块, 可以旋转后放入地图中, 必须连接到已经存在的其它 tile 边上

游戏初始化的时候摆下的第一个 tile 的坐标是 (0, 0), 它的左右两边的坐标分别是 (-1, 0), (1, 0), 它的上下两块的坐标分别是 (0, 1), (0, -1)

tile 旋转
 * rotation % 4 == 0 -> 无旋转
 * rotation % 4 == 1 -> 顺时针 90 度 
 * rotation % 4 == 2 -> 顺时针 180 度 
 * rotation % 4 == 3 -> 顺时针 270 度

== terra ==

地形有数种, 包括 ROAD/FIELD/CITY/CLOISTER, 扩展包可能还包括河流等, 每一个 tile 上包含一种到数种地形

terra 有可能闭合或尚未闭合, 通过 tiepoint (每个 tile 每一边共有三个 tiepoint) 接合, 可以查询与之相邻的其它 terra 组 (例如农田旁的城市)

tiepoint 跟 tile 坐标的关系如下图:

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

推而广之即

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


"""

FIELD = 1

ROAD = 2

CITY = 3

CLOISTER = 4


class Terra(object):
    
    def __init__(self, terra_type, size, open_tiepoint, adjacent):
        self.terra_type = terra_type                # FIELD/ROAD/CITY/CLOISTER
        self.size = size                            # how many tiles are there on this terra
        self.open_tiepoint = set(open_tiepoint)     # all open tiepoint coord
        self.adjacent = set()
        for a in adjacent:
            assert a.terra_type != terra_type
            self.adjacent.add(a)                    # adjacent groups

    def merge(self, t):
        assert self.terra_type == t.terra_type
        self.size += t.size
        self.open_tiepoint ^= t.open_tiepoint
        self.adjacent |= t.adjacent

    @property
    def closed(self):
        return len(self.open_tiepoint) == 0


class Tile(object):

    def __init__(self, coord, rotation=0):
        pass

    @property
    def terras(self):
        pass
        

