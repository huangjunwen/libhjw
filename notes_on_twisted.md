#summary twisted 笔记



# defer #

## defer ##

就其构造而言, 如果简化掉 errback, defered 其实就是一串 callback 链, callback 之间有一个上下文在传递
```
    /*
                res            res            res
    +---------+      +-------+      +-------+     +-------+
    | defered | -->  |  cb1  | -->  |  cb2  | --> |  cb3  |
    +---------+      +-------+      +-------+     +-------+ 
    */
```

例如
```
    # ...
    d = LongOpSoRetDefer()
    d.addCallback(cb)
```
似乎相当于声明了一个独立的控制流(非抢占的, 在需要长时间等待时交出cpu)
```
    def f():
        res = LongOpSoRetDefer()
        cb(res)
        ...
```
所不同之处在于, 前者的调度似乎更**显式**一些, 但后者的可读性更强. 下面的 inlineCallbacks 即使用标准 python 的语法来近似地实现这一想法.


## inlineCallbacks ##

想像一个例子, 在一个连接中需要循环地处理多个请求(如http 1.1的长连接), 如下写法要自然多了: 我们创建一个会运行"很久"的
函数, 期间会有很多次的停顿(例如IO操作)和循环, 因此使用 Generator 的特性把控制交出来, 并且在运行结束之后触发某个函数执行.
```
    @inlineCallbacks
    def rwLoop(src):
        while True:
            r = yield waitOnSomeSource(src)         # waitOnSomeSource return a defer
            # ... some transform on r
            yield writeToSink(r)                    # writeToSink also return a defer
            # ... 
            if someCond:
                break

    # ... got a src (maybe a socket)
    d = rwLoop(src)                                # d is also a defer
    d.addCallback(someWorkAfterLoop)
```

这是个好东西, 所以整个地分析一下 (location: _internet.defer_)

inlineCallbacks 最终实际上调用的是 _inlineCallbacks
```
    def _inlineCallbacks(result, g, deferred):                                          
        """  
        See L{inlineCallbacks}.
        """
        # This function is complicated by the need to prevent unbounded recursion
        # arising from repeatedly yielding immediately ready deferreds.  This while
        # loop and the waiting variable solve that by manually unfolding the
        # recursion.

        waiting = [True, # waiting for result?                                                  # C6
                   None] # result

        while 1:                                                                    # C0
            try: 
                # Send the last result back as the result of the yield expression.
                if isinstance(result, failure.Failure):
                    result = result.throwExceptionIntoGenerator(g)
                else:
                    result = g.send(result)                                         # C1
            except StopIteration:                                                               # C51
                # fell off the end, or "return" statement
                deferred.callback(None)
                return deferred
            except _DefGen_Return, e:                                                           # C52
                # returnValue call
                deferred.callback(e.value)
                return deferred
            except:                                                                             # C53
                deferred.errback()
                return deferred

            if isinstance(result, Deferred):                                        # C2
                # a deferred was yielded, get the result.
                def gotResult(r):
                    if waiting[0]:
                        waiting[0] = False                                                      # C6
                        waiting[1] = r
                    else:
                        _inlineCallbacks(r, g, deferred)                                        # C7

                result.addBoth(gotResult)                                           # C3
                if waiting[0]:
                    # Haven't called back yet, set flag so that we get reinvoked
                    # and return from the loop
                    waiting[0] = False                                                          # C8
                    return deferred                                                 # C4

                result = waiting[1]                                                             # C9
                # Reset waiting to initial values for next loop.  gotResult uses
                # waiting, but this isn't a problem because gotResult is only
                # executed once, and if it hasn't been executed yet, the return
                # branch above would have been taken.


                waiting[0] = True
                waiting[1] = None


        return deferred
```_

一开始第一次调用时, result 为 None; g 为刚刚构造好的 generator; deferred 也是刚刚构造好的 defer.

执行时最常见的代码执行路径是 **C0 ~ C4**,

**C0** 循环
> 意义: 每一个循环执行一次 g 中一部分代码, 此函数本身的意义也是每次执行 g 中的一部分代码, 由于下面将会讲到的原因而增加了这个 while 循环
**C1** 执行 generator 中的一段程序, 然后切换出来, 扔出来的 result 一般是个 defer
> 意义: 将 g 等待到的结果交给 g, 同时得到 g 将要等待的下一个目标 (another defer)
**C2** 如果 result 不是 defer, 继续循环(又扔回去, 此时 ` x = yield some_expr ` 相当于 ` x = some_expr `), 否则进入 **C2**
> 意义: 现在就能得到的值就不要抛出来啦
**C3** result(defer) 增加一个回调 gotResult, 此回调实际有用功又是 `_inlineCallbacks` 自己
> 意义: 见 C1
**C4** 返回 deferred , 此 deferred 将会在此 generator 运行完之后被回调, 见 **C51 ~ C53**

之所以有 **C6** 这个的存在, 缘由在一开始的注释中有, 这里具体讲就是: 首先在 **C3** 处 addBoth 的时候, 假如返回的 result 是'假'defer(即其实已经有结果了),
addBoth 的时候就会直接执行 gotResult 的代码, 而 gotResult 实际又是执行 `_inlineCallbacks` 的, 假如持续 yield 出来的都是这种 defer 的话, 则
不断的 `_inlineCallbacks` 会导致 stackoverflow.

因此 gotResult 改了一下, 使得执行完 **C3** 之后, 能够得知 gotResult 是否已经执行过, 若是, 则把 result 取出(**C9**)重新循环(递归变循环); 若否, 则返回 defered 留待以后执行.

## trap ##

应该是有一些使用上的陷阱的, 将来补充于此.

# zope.interface #

## about ##

twisted 这么多的组件库是用 zope.interface 来组织的, 这个包主要由 interface 和 adapter 组成, 以前一直觉得所谓 interface 就是一些声明而已, 没有太多实质性意义; 但转过头来想, 其实是正常的, 因为它本身就不关心实质性的工作; 它其实更多的像是一种给程序员的指引(非强制性的, 仅仅是额外的标注), 让程序员在实现代码的时候只需将关注点放在接口上即可; 同时耦合的部分(即接口自己)可以独立提取
出来放在一起, 使程序更加容易理解.

首先可以看一下这个文档 [human.txt](http://libhjw.googlecode.com/svn/wiki/notes_on_twisted.attach/human.txt), 举了例子介绍怎么用.

### interface ###

zope.interface 中的 [README.txt](http://libhjw.googlecode.com/svn/wiki/notes_on_twisted.attach/README.txt):

> Interfaces are objects that specify (document) the external behavior
> of objects that "provide" them.  An interface specifies behavior
> through:

> - Informal documentation in a doc string

> - Attribute definitions

> - Invariants, which are conditions that must hold for objects that
> > provide the interface

interface 提供的是对一组相关属性/功能的"面向人类"的描述, 这组相关的功能通常由某个类(class)或工厂(factory)实现(implement), 而这个类的实例则提供(provide)了这组相关的功能:


> provide
> > We say that objects **provide** interfaces.  If an object provides an
> > interface, then the interface specifies the behavior of the
> > object. In other words, interfaces specify the behavior of the
> > objects that provide them.


> implement
> > We normally say that classes **implement** interfaces.  If a class
> > implements an interface, then the instances of the class provide
> > the interface.  Objects provide interfaces that their classes
> > implement.  (Objects can provide interfaces directly,
> > in addition to what their classes implement.)


> It is important to note that classes don't usually provide the
> interfaces that they implement.

> We can generalize this to factories.  For any callable object we
> can declare that it produces objects that provide some interfaces
> by saying that the factory implements the interfaces.

以上相当啰嗦, 不过区分清楚术语也很重要, 否则那些函数名就看不明白了.


### adapter ###

zope.interface 中的 [adapter.txt](http://libhjw.googlecode.com/svn/wiki/notes_on_twisted.attach/adapter.txt):

> Adapter registries provide a way to register objects that depend on
> one or more interface specifications and provide (perhaps indirectly)
> some interface.  In addition, the registrations have names. (You can
> think of the names as qualifiers of the provided interfaces.)

适配器实际上很形象, 就是传入一些提供(provide)了某些接口的对象, 给你返回一个提供(provide)了另一个接口的对象.

## refs ##

zope 3 wiki:
  * [<A Comprehensive Guide to Zope Component Architecture>](http://libhjw.googlecode.com/svn/wiki/notes_on_twisted.attach/zca.pdf) pdf 电子书
  * http://wiki.zope.org/zope3/ZopeInAnger#interfaces
  * http://wiki.zope.org/zope3/ZopeInAnger#adapters
  * http://wiki.zope.org/zope3/ZopeGuideInterfaces
  * http://wiki.zope.org/zope3/ComponentArchitecture

# concrete #

## twisted.web ##

http.py 中主要包含了跟 http 协议相关的内容, 主要包括:

  * `HTTPFactory` 协议工厂类, twisted 中所有协议都必须生产自一个工厂(factory->protocol, factory 是全局的, protocol 是 per-transport 的)
  * `HTTPChannel` 是协议面向 socket 连接的部分, `Request` 是每一个请求(也包含一些协议的解析工作), 这两个类比较"交错".
  * `HTTPChannel` 有一个类成员 `requestFactory`: 用于构造一个个独立的请求对象, 默认就是 `Request` 类
  * `HTTPChannel` 对象有一个成员 `requests`: 由于 http 1.1 允许同一个 channel 上连续输送多个请求, 需要 `requests` 队列, `requests[0]` 是当前处理的请求, `requests[-1]` 是最后接收到的请求 ~~(但我怀疑其实是没有用的, 原因下述...)~~
  * `HTTPChannel` 工作流程: 它是一个 `LineReceiver`, 当 `lineReceived` 第一行时用 `requestFactory` 构造一个请求对象出来, 然后接收其他头部, 若有 body 则设置为 raw mode 接受 body 数据; 完成后调用 `allContentReceived` 重置所有变量(准备给下一个请求使用), 这个函数再调用 `req.requestReceived` 让 req 对象把剩下的东西也解析完, 接着 `requestReceived` 再调用 `req.process`. 所以整个流程是循环进行的, `HTTPChannel` 接收并构造出请求, 请求接受完就立即处理, 完了之后才又轮到 channel 接收下一个请求. ~~因此说 `requests` 队列是象征性的.~~
  * ~~另外有个比较严重的问题, 若有多个请求, 第一个请求 process 过程中阻塞在 defer 上, 而第二个请求却立马完成, 那么**返回的第一个 resp 将会是第二个请求的**...~~
  * 上面的说法错误了, 实际上是有处理这样的情况的: `Request` 类有一个 `queued` 的变量表示这个请求是队列中的第一个还是后面的, 假如是后面的请求, `req.transport` 指向一个 `StringIO` 而不是直接输出到 `HTTPChannel` 上, 当 `req.finish` 被调用的时候, 会检查这个 `queue` 变量, 若是队列中第一个才做清理工作, 否则不作处理, 留待它前面的都输出了才会重新被唤起输出到实际的 channel 中(`Request.noLongerQueued`)
  * 当然其实还是有瑕疵的, 假如上面那种情况下, 第二个请求的输出非常的多, 那么内存是会一直消耗着的, 当然这样会换来性能上的好处(因为多个请求不是串行地处理)

server.py 则是在 http.py 之上对 `Resource` 的封装, 提出了几个概念, 最终用户可能大部分情况下都是跟这些打交道:
  * `Site`: 继承自 `http.HTTPFactory`, 添加了 Resource tree 的概念
  * `Resource`: URL 上的某一节, 代表资源的层级关系

websocket.py 目前只在 [websocket-4173](http://twistedmatrix.com/trac/browser/branches/websocket-4173) 这个分支上, 新添加了新类型的 `Resource`, 它上面需要添加一个 handler, 处理 websocket 两端来往的 frame.. (个人觉得 websocket 协议真如其名, 伪装成为一个 http 请求连上来, 特别猥琐....)