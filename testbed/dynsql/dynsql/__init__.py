from const import *
from env import Env
from dynsql import Dynsql


"""
Common usage:
>>> from dynsql import *
>>>
>>> Env.get("MySQLdb").as_default()
>>> d = Dynsql("select ?(select, 'x, y, z') from ?tab limit {$offset, } $row_count")
>>>
>>> d(select='*', tab='invoice', row_count=20) 
('select * from invoice limit  %s', [20])
>>>
>>> d.specialize(select="*", offset=Nil)
<Dynsql 'select * from ?(tab) limit  $(row_count)'>
"""
