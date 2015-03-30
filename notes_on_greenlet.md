#summary greenlet 实现原理

颇花了一段时间终于大致看明白了 greenlet 的代码, 还好有注释帮助

每一个 greenlet 实际上是一段有效的 c stack, 如果某 greenlet 是当前线程中那个运行着的 greenlet 的话, 则它的 stack
会"整个"地置于 c stack 上, 否则如果它曾经执行过(现在被suspend了)的话, 则它的部分或全部 stack 会被移到 heap 上.


#### explanation in code ####

首先应该看一下它代码里的解释

```c

/*
A PyGreenlet is a range of C stack addresses that must be
saved and restored in such a way that the full range of the
stack contains valid data when we switch to it.

Stack layout for a greenlet:

               |     ^^^       |
               |  older data   |
               |               |
  stack_stop . |_______________|
        .      |               |
        .      | greenlet data |
        .      |   in stack    |
        .    * |_______________| . .  _____________  stack_copy + stack_saved
        .      |               |     |             |
        .      |     data      |     |greenlet data|
        .      |   unrelated   |     |    saved    |
        .      |      to       |     |   in heap   |
 stack_start . |     this      | . . |_____________| stack_copy
               |   greenlet    |
               |               |
               |  newer data   |
               |     vvv       |


Note that a greenlet's stack data is typically partly at its correct
place in the stack, and partly saved away in the heap, but always in
the above configuration: two blocks, the more recent one in the heap
and the older one still in the stack (either block may be empty).

Greenlets are chained: each points to the previous greenlet, which is
the one that owns the data currently in the C stack above my
stack_stop.  The currently running greenlet is the first element of
this chain.  The main (initial) greenlet is the last one.  Greenlets
whose stack is entirely in the heap can be skipped from the chain.

The chain is not related to execution order, but only to the order
in which bits of C stack happen to belong to greenlets at a particular
point in time.

The main greenlet doesn't have a stack_stop: it is responsible for the
complete rest of the C stack, and we don't know where it begins.  We
use (char*) -1, the largest possible address.

States:
  stack_stop == NULL && stack_start == NULL:  did not start yet
  stack_stop != NULL && stack_start == NULL:  already finished
  stack_stop != NULL && stack_start != NULL:  active

The running greenlet's stack_start is undefined but not NULL.
*/

```


#### greenlet definition ####

```c

    typedef struct _greenlet {
        PyObject_HEAD
        char* stack_start;
        char* stack_stop;
        char* stack_copy;
        long stack_saved;
        struct _greenlet* stack_prev;
        struct _greenlet* parent;
        PyObject* run_info;
        struct _frame* top_frame;
        int recursion_depth;
        PyObject* weakreflist;
    } PyGreenlet;

```

stack\_start/stop/copy/saved 这几个字段在 [上边](#explanation_in_code.md) 有说明

stack\_prev 指的是在 c stack 上的先后关系

parent 是指创建此 greenlet 的那个 greenlet(非执行的那个), 当某 greenlet 执行完成之后, 控制流会返回到 parent 处,
实际上在此实现中, parent 是哪一个都可以的, 甚至动态修改, 所有 greenlet 之间都可以互相切换

run\_info 运行对象


#### greenlet initialization ####

填充 greenlet run\_info 以及 parent, stack\_stop/start 均为 NULL


#### greenlet run ####

_注: 下文以 **target** 表示将要跳转去的 greenlet, **current** 表示当前 greenlet_

当在某个 greenlet 中 调用 target.switch(xxx) 的时候, 控制流就会去到 target 中, 分几种情况:
  1. target [尚未执行过](#greenlet_not_stated)
  1. target [曾经执行过但已暂停](#greenlet_resume)
  1. target [已经执行完](#greenlet_dead)

见函数 **g\_switch**

```c

    // ...
    while (1) {
        if (PyGreen_ACTIVE(target)) {
            ts_target = target;
            _PyGreen_switchstack();
            return ts_passaround;
        }
        if (!PyGreen_STARTED(target)) {
            void* dummymarker;                      
            ts_target = target;
            _PyGreen_initialstub(&dummymarker);     // 取当前 frame 中的一个变量的地址
            return ts_passaround;
        }
        target = target->parent;                    // dead
    }
    // ...

```

##### greenlet not stated #####

实际调用的函数是 `_PyGreen_initialstub`
```c

    //...
    /* start the greenlet */
    ts_target->stack_start = NULL;          
    ts_target->stack_stop = (char*) mark;   // 这个是 caller 中一个变量的地址(见 g_switch 代码), 取这个之上
                                            // 到堆栈顶作为 target greenlet 的 c stack

    // ...
    err = _PyGreen_switchstack();           // fork 堆栈!!
    /* returns twice!
       The 1st time with err=1: we are in the new greenlet
       The 2nd time with err=0: back in the caller's greenlet
    */
    if (err == 1) {
        /* in the new greenlet */
        // ...
        result = PyEval_CallObject(run, args);      // 运行
        // ...

        /* jump back to parent */
        ts_self->stack_start = NULL;  /* dead */
        g_switch(ts_self->parent, result);

        // 不应该来到这里
    }
    // 返回到 caller

```

`_PyGreen_initialstub` 示意图如下, 新的 greenlet(target) 的堆栈肯定在 dummymarker 之下, 所以可以用这个作为 target->stack\_stop (包含这部分以下的堆栈即可正确运行 target)

```c

        /*

        |     older functions     |   
        |_________________________|   
        |                         |
        |        g_switch         |  
        |                         |   <- &dummymarker in this frame    . . ____________ . . target->stack_stop
        |_________________________|                                       |            |
        |                         |                                       |            |
        |  _PyGreen_initialstub   |                                       |            |
        |_________________________|                                       |            |   
        |                         |                                       |            |
        |  _PyGreen_switchstack   |                                       |            |
        |_________________________|                                       |            |   
        |                         |                                       |            |
        |  _PyGreen_slp_switch    |                                       |            |
        |_________________________|   <- ESP                          . . |____________| . . current->stack_start
        |                         |                                                     
        |                         |                                                     
        |        not used         |                                                     

        */

```

第一次对某个 greenlet 运行 `_PyGreen_slp_switch` 并不会有 stack switch 的操作, 而是直接返回 1 (见下面代码), 所以第一次 `_PyGreen_switchstack` 也就返回1而直接进入 target greenlet 执行.
返回1之前还有一个操作: `slp_save_state` 将 ESP ~ target->stack\_stop 之间的内容作为 current 的部分 stack 移到堆上(见上图右方那一块), 这也是为什么 `_PyGreen_switchstack` 会返回两次的原因, 因为当控制重新返回 current 的时候, 这部分属于它的 heap 上的 stack 会重新拷贝回 c stack 继续执行, 于 current 而言, 效果同样也是从 `_PyGreen_switchstack` 返回, 只是返回值不一样.

```c

    // _PyGreen_slp_switch 部分代码(实际上是在 SLP_SAVE_STATE 这个宏中)
    
    // ...

    if (slp_save_state((char*)stackref)) return -1;
    if (!PyGreen_ACTIVE(ts_target)) return 1;

    // stack switch here

```

##### greenlet resume #####

`_PyGreen_slp_switch` 可以这样表示

```c

    // asm 代码用于将需要保存的寄存器放到 stack 上, 并将 ESP 赋给 stackref
    
    if (slp_save_state((char*)stackref)) return -1;     // cp c stack 上的内容(可能包含多个 greenlet)一直到 target->stack_stop 为止
    if (!PyGreen_ACTIVE(ts_target)) return 1;           // 见上一节
    
    // asm 代码将 ESP 指向 target->stack_start

    slp_restore_state();                                // 将 target 中的存盘(stack_saved)拷贝回 c stack

    // asm 代码恢复寄存器
   
```

问题: slp\_save\_state 的时候, 如何知道当前 c stack 上有哪些 greenlet 要 swap 到 heap 上呢? 这个就是 **stack\_prev** 所起的作用了.

##### greenlet dead #####

如果 target 已执行完, 则 target 变为 target->parent, 一直下去, 直到找到可以执行的 greenlet 为止, 然后按照上述两种情况执行.
这里可以看到, 虽然 parent 可以随便设, 但是要保证一点不可以出现 cycle, 这个可见函数 green\_setparent.

#### invariant ####

每一个 greenlet 从创建开始到运行结束只要保证它的有效堆栈始终完好的即可.

#### not cover ####

还有一些细节例如 main greenlet 如何创建, 怎么样 kill 一个 greenlet 等等.
