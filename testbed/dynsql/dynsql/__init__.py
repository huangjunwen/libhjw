from const import *
from env import Env
from dynsql import Dynsql


"""
Common usage:
>>> from dynsql import Env, Dynsql
>>>
>>> Env.get("MySQLdb").as_default()
>>> d = Dynsql("select ?(abc, 'x, y, z') from ?tab limit [$offset, ] $row_count")
>>>
>>> d({'abc': '*', 'tab': 'invoice', 'row_count': 20}) 
('select * from invoice limit  %s', [20])
"""
