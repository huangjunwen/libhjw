#lua 笔记

#### False value ####

除了 nil 跟 false 之外, 其余都是真值.

#### Table form ####

table 类型的最通用的形式是

```lua

    a = {[0] = 1, [2] = 2, ["x"] = "yyy"}

```

因为 `a.x` 等价于 `a['x']`    (这点跟 js 比较像)

也可以用 ';' 分隔列表部分跟字典部分
```lua

    a = {'a', 'b', 'c'; x = 1, y = 2, z = 3}

```

列表下标从 1 开始

#### Break & return ####

控制流中的 break/return 必须是在 block 的最后出现.


#### Function decl & function invok ####

函数可以以以下几种方式声明
```lua

	function normal(a, b, c) body end
	function variable_arg(a, b, c, ...) body end

```
函数体中可以用 arg (一个 table) 这个变量来访问 `...`

函数可以以以下几种方式调用
```lua

	f(a, b, c)
	f{a='x', b='c'}

```
后者函数会接收到一个单参.

函数可以返回多个值, 通过 `{}` 来打包多个值;  通过 `unpack` 来解包:
```lua

	function f()
	  return 1, 2
	end
	t = {f()}

	a, b, c = unpack(t)
	print(a)
	print(b)
	print(c)

```

多值赋值 (例如调用函数时的参数绑定或者 `a, b = f()`) 都不会出错 (不像 python, 两边的数量必须一致), 多余的都赋值 nil.


#### Scopes ####

```lua

    -- non-global functions

    local function f (...) {
      ...
    }
    
    -- is equivalent to
    
    local f
    function f (...) {
    }

```

为什么有这样的东西呢? 因为在一个函数定义的过程中, 这个函数名尚未有效, 例如:

```lua

    local fact = function (n)
      if n == 0 then return 1
      else return n*fact(n-1)   -- buggy
      end
    end

```


#### Tail Calls ####

lua 能正确地处理尾递归. 很适合写状态机, 例如:

```lua

    function room1 ()
      if ... then return room2()
      elseif ... then return room3()
      else
        return room1()
      end
    end
    
    function room2 ()
      if ... then return room3()
      else
        return room2()
      end
    end

    function room3 ()
      print("you win")
    end
    
```

#### Generic for ####

实际上 lua 中的 generic for

```lua

    for var_1, ..., var_n in explist do 
      block 
    end

```

语义上等价于

```lua

    do
      local f, s, var = explist                  -- code 1
      while true do
        local var_1, ... , var_n = f(s, var)
        var = var_1
        if var == nil then break end
        block
      end
    end

```

code 1 处 explist 称为 iterator factory; 三个变量分别分别称为 iterator function/invariant state/control variable.
factory 构造出这三个东西之后, 每次都运行 iterator function, 传给它 invariant state 以及上一次的 control variable 产生一堆变量.
其中第一个就是下一个 control variable; 一直循环到这个控制变量为 nil 为止.

如果使用 function closure 来实现 iterator 的话, 例如

```lua

    function list_iter (t)
      local i = 0
      local n = table.getn(t)
      return function ()
               i = i + 1
               if i <= n then return t[i] end
             end
    end

```

则 code 1 处 f 为返回的函数, s 以及 var 都是 nil (这些由 closure 内部管理了)
了解了 for 的语义之后, 完全可以不使用 closure 来实现 iterator (更高的效率). 例如:

```lua

    function iter (a, i)
      i = i + 1
      local v = a[i]
      if v then
        return i, v
      end
    end
    
    function ipairs (a)
      return iter, a, 0
    end

    a = {"one", "two", "three"}
    for i, v in ipairs(a) do
      print(i, v)
    end

```
