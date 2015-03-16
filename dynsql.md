#summary dynsql 文档

# 介绍 #

dynsql 是一个 SQL 语句的模板库, 用于动态地构造 SQL, 并非 ORM.

一般用于构造 SQL 语句的工具都会试图去"理解" SQL 的各部分, 例如 select field/table reference/where ... 等等, 这样做对于那些符合这种"理解"的语句能够变得非常易于编写, 但超出工具"理解"的那些往往要 hard code 了.

dynsql 尝试使用另外一种方法, 它完全不理解交给它处理的实际内容, 而仅仅处理那些它认得的标志(就像网页模板一样), 例如在 mysql 中, 限定选出的行数:

```
    >>> d = DynSql("limit {$offset,} $row_count")
    >>>
    >>> d(row_count=20)
    ('limit  %s', (20,))
    >>>
    >>> d(row_count=20, offset=10)
    ('limit %s, %s', (10, 20))
```

由于它不"理解"它处理的东西, 所以它可以用于所有简单/复杂情况.

另外有一个额外的好处, 构造效率只与模板中的标志的复杂度有关, 而跟 SQL 语句的复杂度无关, 下面详解.

## 代码 ##

browse: _http://code.google.com/p/libhjw/source/browse/#svn/trunk/testbed/dynsql_

checkout: _https://libhjw.googlecode.com/svn/trunk/testbed/dynsql_

# 标记符 #

dynsql 定义的标记符比较简单, 共有以下几类:

| 类型 | 例子 | 说明 |
|:-------|:-------|:-------|
| var 变量替换符 | `$a`, `$(a)` |  |
| raw 原始替换符 | `?a`, `?(a)`, `?(a, "xyz")` | 直接替换到 SQL 语句中 |
| cntrl 控制符 | `#if(a)`, `#ifn(a)` |  |
| sub 子节点 | `{$offset,}` | `{` 跟 `}` 包围的部分 |
| 文本 | `'limit '` | 除了上边那些标记符, 其余都是文本 |

_注: `$`/`?`.. 这些可以配置改变使用其它符号, 因为例如 ODBC 中 '?' 是有特殊意义的_

`{` 以及 `}` 表达的是层次关系, 可以嵌套, 所有标记符的作用域都在当前的 `{...}` 中.

所以标记符按照 left-2-right, depth-first 方式遍历,  在一个 `{...}` 中, 若任何一个标记符(除了它的子节点)求值为 Nil, 则整个 `{...}` 返回 Nil.

见一开始的例子, `offset` 存在的时候, 内层(包括那个逗号)有值, 所以结果返回 `'limit %s, %s'`, 不存在的时候返回 `'limit  %s'`.

mysql 5.0 中 select 的语法定义:
```
    SELECT
        [ALL | DISTINCT | DISTINCTROW ]
          [HIGH_PRIORITY]
          [STRAIGHT_JOIN]
          [SQL_SMALL_RESULT] [SQL_BIG_RESULT] [SQL_BUFFER_RESULT]
          [SQL_CACHE | SQL_NO_CACHE] [SQL_CALC_FOUND_ROWS]
        select_expr, ...
        [FROM table_references
        [WHERE where_condition]
        [GROUP BY {col_name | expr | position}
          [ASC | DESC], ... [WITH ROLLUP]]
        [HAVING where_condition]
        [ORDER BY {col_name | expr | position}
          [ASC | DESC], ...]
        [LIMIT {[offset,] row_count | row_count OFFSET offset}]
        [PROCEDURE procedure_name(argument_list)]
        [INTO OUTFILE 'file_name' export_options
          | INTO DUMPFILE 'file_name'
          | INTO var_name [, var_name]]
        [FOR UPDATE | LOCK IN SHARE MODE]]
```

简化后可以这样表达:
```
    select_base = DynSql("""
        SELECT ?select 
            {FROM ?tab 
                {WHERE ?where} 
                {GROUP BY ?group_by {HAVING ?having}} 
                {ORDER BY ?order_by} 
                {LIMIT {$offset,} $(limit)} 
                {#if(for_update) FOR UPDATE}
            }
    """)
```

# 用法 #

用法很简单:
```
    >>> from dynsql import *
    >>>
    >>> init("MySQLdb")
    >>> d = DynSql("select ?(select, 'x, y, z') from ?tab limit {$offset, } $row_count")
    >>>
    >>> d(select='*', tab='invoice', row_count=20) 
    ('select * from invoice limit  %s', [20])
    >>>
    >>> d.specialize(select="*", offset=Nil)
    <DynSql 'select * from ?(tab) limit  $(row_count)'>
```

首先初始化 `init("MySQLdb")`, 这一句会创建一个默认的 `Env` 环境对象, 环境提供的是各种配置, 包括标记符使用的符号, 控制符字典等等.

`DynSql` 构造时需要一个 `Env` 对象, 可以显式传入, 也可以不传, 后一种情况就会使用默认的, 在此例子即使用 MySQLdb 环境.

`DynSql` 对象主要有两个方法, 第一是 `__call__`, 根据传入的参数构造出最终的 SQL 语句以及参数; 另一个是 `specialize`, 这个方法做的事情是根据给定的参数去掉一部分的"灵活性", 返回一个新的特殊化了的 `DynSql`.

例如 `select_base` 表示的是 select 语句最灵活的情况, 当 select 的是某个特定的表的时候, 我们认为它丧失了部分的灵活性.
```
    >>> print select_base
    SELECT ?select 
        {FROM ?tab 
            {WHERE ?where} 
            {GROUP BY ?group_by {HAVING ?having}} 
            {ORDER BY ?order_by} 
            {LIMIT {$offset,} $(limit)} 
            {#if(for_update) FOR UPDATE}
        }
    >>>
    >>> some_tab = {'select': 'id, name, province', 'tab': 'user'}
    >>> print select_base.specialize(some_tab)
    SELECT id, name, province 
        FROM user 
            {WHERE ?(where)} 
            {GROUP BY ?(group_by) {HAVING ?(having)}} 
            {ORDER BY ?(order_by)} 
            {LIMIT {$(offset),} $(limit)} 
            {#if(for_update) FOR UPDATE}

```

丧失部分灵活性的时候, 可能嵌套的层次就少了, 某些 `{...}` 消失了, 某些变量变成了常量, 此时执行 `__call__`, 效率是比原来的更高的, 所以构造很复杂的语句的时候, 可能会反而效率更高.

_注: 代码还不稳定, 接口有可能会变化_

# 控制符 #

控制符可以用来实现一些特殊的行为:

| 控制符 | 例子 | 说明 |
|:----------|:-------|:-------|
| `if` | `#if(a)` | 若变量 `a` 真时通过, 否则当前 `{...}` 返回 `Nil` |
| `ifn` | `#ifn(a)` | if 的反义 |
| `mutex`, `mtx`, `m` | `#m(a)` | 互斥量, 只有一个 `{...}` 能够获得名为 `a` 的互斥量 |
| `succ` | `#succ(a)` | 当 `#succ(a)` 所处的 `{...}` 有值时, `a` 置为 `True` |
| `fail` | `#fail(a)` | 当 `#fail(a)` 所处的 `{...}` 为 `Nil` 时, `a` 置为 `True` |
| `nil`, `err` | `#nil()` | 总是返回 `Nil`, 一般用于测试? |

一些例子:
```
    >>> d = DynSql("{#m(x) a=$a}{#m(x) b=$b}{#m(x) 1=1}")
    >>> d()
    (' 1=1', ())
    >>> d(b=3)
    (' b=%s', (3,))
```

注意下面这个例子, 其结果可能跟预期不一样:
```
    >>> d = DynSql("{{#succ(A) this is A} #nil()} {#if(A) A is NotNil}") 
    >>> d()
    ('  A is NotNil', ())                    # A is Nil in fact
```

由于 'seg A' 的父节点返回 `Nil`, 所以虽然 `'{#succ(A) this is A}'` 是成功的, 但这一部分还是没有显示出来. 可以这样补救:
```
    >>> d = DynSql("{{#succ(A) this is A} #nil() #succ(B)} {#if(A) #if(B) A is NotNil}") 
    >>> d()
    (' ', ())
```


# TODO #

还有很多事情需要做, 首要的是确定如何组织模板, 例如以树状的形式: 根是 `select_base`/`insert_base` 等等

# 设计 #