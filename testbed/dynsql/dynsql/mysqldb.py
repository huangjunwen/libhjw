from core.base import *
from core.env import Env
from core.dynsql import DynSql

Env('MySQLdb', '%s', '?', '$', '@', '#', '{', '}').as_default()

# SELECT_STMT = DynSql(
#     "SELECT ?(select_expr) {FROM ?(tab_ref)"\
#     " {WHERE ?(where_cond)}"\
#     " {GROUP BY ?(group_by) {HAVING ?(having_cond)}}"\
#     " {ORDER BY ?(order_by)}"\
#     " {LIMIT {$(limit_offset), }$(limit_cnt)}"\
#     " {#if(for_update) FOR UPDATE}}")
# 
# # currently insert/update/delete only support single table (use tab_name variable)
# # later may add multiple-table support, but i think they are rarely used.
# 
# INSERT_STMT = DynSql(
#     "{#if(replace)REPLACE}{#ifn(replace)INSERT} INTO ?(tab_name)"\
#     " {(?(col_names))}"\
#     " {#ifn(select_stmt) VALUES ?(insert_values)}"\
#     " {?(select_stmt)}"\
#     " {#ifn(replace) ON DUPLICATE KEY UPDATE ?(update_values)}")
# 
# INSERT_SIMPLE_STMT = INSERT_STMT.specialize(replace=Nil, select_stmt=Nil, 
#     on_dup=Nil, 
#     insert_values=NotNil)
# 
# UPDATE_STMT = DynSql(
#     "UPDATE ?(tab_name) SET ?(update_values)"\
#     " {WHERE ?(where_cond)}"\
#     " {ORDER BY ?(order_by)}"\
#     " {LIMIT $(limit_cnt)}")
# 
# DELETE_STMT = DynSql(
#     "DELETE FROM ?(tab_name)"\
#     " {WHERE ?(where_cond)}"\
#     " {ORDER BY ?(order_by)}"\
#     " {LIMIT $(limit_cnt)}")
