彻底的 CPS , 函数调用不是所谓的 subroutine , 而是控制流的转移, 如:

```
    def f(x, y):
        return x**2 + y
```

在 CPS 中的形式

```
    def power(v, e, c):
        c(v**e)
    
    def add(l, r, c):
        c(l+r)

    def f(x, y, c):
        power(x, 2, lambda x2: add(x2, y, c))
```

learned keywords: continuation CPS

#### links ####

  * http://en.wikipedia.org/wiki/Continuation Continuation
  * http://en.wikipedia.org/wiki/Continuation-passing_style CPS 比之 subroutine 是更为 general 的方式
  * http://en.wikipedia.org/wiki/Coroutine
  * http://en.wikipedia.org/wiki/Setcontext  POSIX 的系统调用哈~~* http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html 这个没有用到那些 stack unwind 的操作, 不过这代码真学习了...
```
switch (count % 8) {
    case 0:        do {  *to = *from++;
    case 7:              *to = *from++;
    case 6:              *to = *from++;
    case 5:              *to = *from++;
    case 4:              *to = *from++;
    case 3:              *to = *from++;
    case 2:              *to = *from++;
    case 1:              *to = *from++;
                   } while ((count -= 8) > 0);
}
```
  * http://www.sics.se/~adam/pt/ 实现一
  * http://www.dekorte.com/projects/opensource/libcoroutine/ 实现二
  * http://software.schmorp.de/pkg/libcoro.html 这个更轻量级
  * http://www.crystalclearsoftware.com/soc/coroutine/index.html boost的
  * http://xmailserver.org/libpcl.html 还有~~

  * http://www.ps.uni-saarland.de/~duchier/python/continuations.html