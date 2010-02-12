#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8:nu:nowrap

from board import *

class Tile(TileBase):
    spirit_coord = (0, 0)
    terra_proto = [(FIELD, [11, 0], {}), (ROAD, [1,], {}), (FIELD, [2, 3], {}), (ROAD, [4, ], {}),
        (FIELD, [5, 6], {}),
        (ROAD, [7, ], {}),
        (FIELD, [8, 9], {}),
        (ROAD, [10, ], {})]
    adjacent = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 7), (7, 0)]
    amount = 1


class Tile(TileBase):
    spirit_coord = (0, 1)
    terra_proto = [(CITY, range(12), {'shield': 1})]
    adjacent = []
    amount = 1


class Tile(TileBase):
    spirit_coord = (0, 2)
    terra_proto = [(FIELD, range(12), {}), (CLOISTER, [12,], {})]
    adjacent = [(0, 1)]
    amount = 4


class Tile(TileBase):
    spirit_coord = (0, 3)
    terra_proto = [(FIELD, range(0, 7) + range(8, 12), {}), (ROAD, [7,], {}), 
        (CLOISTER, [12,], {})]
    adjacent = [(0, 1), (1, 2), (2, 0)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (0, 4)
    terra_proto = [(FIELD, [11, 0, 1, 2, 3], {}), (ROAD, [4,], {}), (FIELD, [5, 6], {}), 
        (ROAD, [7,], {}), 
        (FIELD, [8, 9], {}),
        (ROAD, [10,], {})]
    adjacent = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 0)]
    amount = 4


class Tile(TileBase):
    spirit_coord = (0, 5)
    terra_proto = [(CITY, range(0, 6) + range(9, 12), {}), (FIELD, [6, 7, 8], {})]
    adjacent = [(0, 1)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (0, 6)
    terra_proto = [(CITY, range(0, 6) + range(9, 12), {'shield': 1}), (FIELD, [6, 7, 8], {})]
    adjacent = [(0, 1)]
    amount = 1


class Tile(TileBase):
    spirit_coord = (0, 7)
    terra_proto = [(CITY, range(0, 6) + range(9, 12), {}), (FIELD, [6,], {}), 
        (ROAD, [7,], {}),
        (FIELD, [8,], {})]
    adjacent = [(0, 1), (0, 2), (0, 3), (1, 2), (2, 3)]
    amount = 1


class Tile(TileBase):
    spirit_coord = (1, 0)
    terra_proto = [(CITY, range(0, 6) + range(9, 12), {'shield': 1}), (FIELD, [6,], {}), 
        (ROAD, [7,], {}),
        (FIELD, [8,], {})]
    adjacent = [(0, 1), (0, 2), (0, 3), (1, 2), (2, 3)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (1, 1)
    terra_proto = [(FIELD, [8, 9, 10, 11, 0], {}), (ROAD, [2, 7], {}), (FIELD, range(3, 8), {})]
    adjacent = [(0, 1), (1, 2)]
    amount = 8


class Tile(TileBase):
    spirit_coord = (1, 2)
    terra_proto = [(CITY, [9, 10, 11, 0, 1, 2], {}), (FIELD, range(3, 9), {})]
    adjacent = [(0, 1)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (1, 3)
    terra_proto = [(CITY, [9, 10, 11, 0, 1, 2], {'shield': 1}), (FIELD, range(3, 9), {})]
    adjacent = [(0, 1)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (1, 4)
    terra_proto = [(CITY, [9, 10, 11, 0, 1, 2], {}), (FIELD, [3, 8], {}), (ROAD, [4, 7], {}), 
        (FIELD, [5, 6], {})]
    adjacent = [(0, 1), (1, 2), (2, 3)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (1, 5)
    terra_proto = [(CITY, [9, 10, 11, 0, 1, 2], {'shield': 1}), (FIELD, [3, 8], {}), 
        (ROAD, [4, 7], {}), 
        (FIELD, [5, 6], {})]
    adjacent = [(0, 1), (1, 2), (2, 3)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (1, 6)
    terra_proto = [(FIELD, range(0, 7) + [11], {}), (ROAD, [7, 10], {}), (FIELD, [8, 9], {})]
    adjacent = [(0, 1), (1, 2)]
    amount = 9


class Tile(TileBase):
    spirit_coord = (1, 7)
    terra_proto = [(FIELD, range(0, 3), {}), (CITY, range(3, 6) + range(9, 12), {}), 
        (FIELD, range(6, 9), {})]
    adjacent = [(0, 1), (1, 2)]
    amount = 1


class Tile(TileBase):
    spirit_coord = (2, 0)
    terra_proto = [(FIELD, range(0, 3), {}), (CITY, range(3, 6) + range(9, 12), {'shield': 1}), 
        (FIELD, range(6, 9), {})]
    adjacent = [(0, 1), (1, 2)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (2, 1)
    terra_proto = [(CITY, range(0, 3), {}), (FIELD, range(8, 12) + [3,], {}), (ROAD, [4, 7], {}),
        (FIELD, [5, 6], {})]
    adjacent = [(0, 1), (1, 2), (2, 3)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (2, 2)
    terra_proto = [(CITY, range(0, 3), {}), (FIELD, range(3, 7) + [11,], {}), (ROAD, [10, 7], {}),
        (FIELD, [8, 9], {})]
    adjacent = [(0, 1), (1, 2), (2, 3)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (2, 3)
    terra_proto = [(CITY, range(0, 3), {}), (FIELD, range(3, 12), {})]
    adjacent = [(0, 1)]
    amount = 5


class Tile(TileBase):
    spirit_coord = (2, 4)
    terra_proto = [(CITY, range(0, 3), {}), (CITY, range(3, 6), {}), (FIELD, range(6, 12), {})]
    adjacent = [(0, 2), (1, 2)]
    amount = 2


class Tile(TileBase):
    spirit_coord = (2, 5)
    terra_proto = [(FIELD, range(0, 3) + range(6, 9), {}), (CITY, range(3, 6), {}), (CITY, range(9, 12), {})]
    adjacent = [(0, 1), (0, 2)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (2, 6)
    terra_proto = [(CITY, range(0, 3), {}), (FIELD, [3, 11], {}), (ROAD, [4,], {}), (FIELD, [5, 6], {}),
        (ROAD, [7,], {}),
        (FIELD, [8, 9], {}),
        (ROAD, [10,], {})]
    adjacent = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1)]
    amount = 3


class Tile(TileBase):
    spirit_coord = (2, 7)
    terra_proto = [(CITY, range(0, 3), {}), (FIELD, [3, 11], {}), (ROAD, [4, 10], {}), 
        (FIELD, range(5, 10), {})]
    adjacent = [(0, 1), (1, 2), (2, 3)]
    amount = 4
    start = True

b = Board()
