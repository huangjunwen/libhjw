#summary 终端笔记

_注: 下文是在 linux 下实践所得, 不过其他 nix 应该差不多_

### 终端简介 ###

  * 历史上真实的 terminal 其实就是一个 **"mon + keyborad + 少量内存 + cable"** 的设备, 不具备实际的计算功能, "tty" 这个词本身即是 teletype (电传打字机)的意思. 而如今我们用的都是 emulattion. (ref http://tldp.org/HOWTO/Text-Terminal-HOWTO-1.html#ss1.7)


### 终端IO ###

#### 设置 ####

  * 用户进程以及实际的终端驱动之间存在着一个终端行规程( terminal line discipline ), 从终端的输入会先经过此而后才进入用户进程(向终端的输出也会先经过此), 这个模块在不同的模式下会对输入输出进行不同的处理(例如在规范模式下, 判断出用户输入的是 Ctrl+C, 因此给用户进程发送一个中断信号, 用户进程无法获得此字符; 而在原始模式下, 用户进程则可以获得此字符).
  * 模式实际上是一组终端设备特性所组成, 这些特性可通过设置 termios 结构的标志位然后通过 tcgetattr/tcsetattr .. 等终端IO函数进行控制.
  * 常用的模式有三种
    * 以行为单位的, 对特殊字符进行处理的模式 (cooked mode)
    * 原始模式, 不以行为单位, 不对特殊字符进行处理 (raw mode)
    * cbreak 模式, 类似原始模式, 但某些特殊字符也进行处理
    * 以下使用 curses 库进行演示, 修改 enter 的参数可看出两种模式下对待特殊字符的不同处理(例如按下 Ctrl+C )
    
```python

    import sys, curses, atexit

    def enter(cbreak=True):
        scr = curses.initscr()                # 初始化 curses
        curses.raw()                        # 先进入 raw 模式
        if cbreak:                            # 是否进入 cbreak 模式
            curses.cbreak()
        scr.clear()                            # 清屏
        scr.refresh()                        # 使清屏生效
        atexit.register(leave, cbreak)        # 注册 atexit handler
        return scr 

    def leave(cbreak):
        if cbreak:
            curses.nocbreak()
        curses.noraw()
        curses.endwin()

    def main():
        enter(False)                        # True 时 cbreak 模式, False 时 raw  模式
        i = 10
        while i > 0:
            print ord(sys.stdin.read(1))
            sys.stdout.flush()
            i -= 1

    if __name__ == "__main__":
        main()

```

  * tcgetattr/tcsetattr 函数需要一个 fd 的参数, 在下面的程序中无论使用标准输入还是标准输出作为这个参数, 行为都是一样的. 这暗示了标准输入/输出其实是同一个文件(结构).

```python

    import sys, tty, termios

    oldtty = termios.tcgetattr(sys.stdin)   # stdin/stdout 都一样
    try:
        tty.setraw(sys.stdout)              # stdin/stdout 都一样
        i = 10
        while i > 0:
            print ord(sys.stdin.read(1))
            i -= 1
    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, oldtty)     # stdin/stdout 都一样

```

#### 终端能力 ####

  * 历史上有众多的 terminal 类型, host compuer 发送到 terminal 不单只有需要显示出来的字, 也包括有控制字符. 早期的只有很少一些, 但后期却有数百个之多; 后来有些终端还有其他各种能力(例如向上滚动, 可以移动光标等), 而定义这些动作的控制字符在不同的终端下也不一样, 因此需要有一个标准: 最终一统江湖的是 termcap (terminal capabilities) 以及其后继者 terminfo 数据库.
  * 终端类型在 host computer 的 TERM 环境变量中 (我们的客户端也需要设置相同的终端类型, 否则两者无法正常沟通)

```bash

    $ echo $TERM
    linux

```

  * 可用 infocmp 命令查询指定终端类型数据库, man terminfo 可知 terminfo 数据库在什么位置, 如下面是 /lib/terminfo/**/**

```bash

    $ infocmp linux
    #       Reconstructed via infocmp from file: /lib/terminfo/l/linux
    linux|linux console,
            am, bce, ccc, eo, mir, msgr, xenl, xon,
            colors#8, it#8, ncv#18, pairs#64,
    ...

```

#### 终端IO总结 ####

  * 终端IO很繁杂, 可能是历史原因.
  * termios 跟 terminfo 之间的关系, 个人觉得 termios 更着重于对输入输出的控制, 例如是否回显输入, 是否理解 Ctrl+C/Ctrl+S 这些控制字符等; 而 terminfo 则更着重于客户端(终端)的表现能力以及沟通协议, 例如什么字符串表示什么颜色之类的; 但两者似乎又有些重合.
  * 用 ncurses 库

### 终端设备 ###

  * 真实的终端已经寥寥, 大多都是伪终端. 但对于用户程序来说是没有区别的.
  * /dev/ttyS0 S1 S2.. 这些是真实的串口设备
  * BSD 伪终端: /dev/ptyXY /dev/ttyXY 前者为主设备, 后者为从设备, 它们之间用管道相连接, 共同模拟一个终端的行为. 主设备主要是用在 rlogin/ssh 这些程序上, 从设备则主要提供给用户程序, 例如应用想要往某个伪终端里写东西, 则可以往从设备里写.
  * Unix 98 伪终端: /dev/pts/x 在文件系统里面只有一个, 这个相当于上边所说的从设备, 主设备实际上是打开文件 /dev/ptmx 获得, 详细看参看手册.
  * 试验
```bash

   jayven@debian:~$ ps ajx
    PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
       ......
       1  1952  1952  1952 ?           -1 S<s      0   0:00 /usr/sbin/sshd
       ......
    1952  3218  3218  3218 ?           -1 S<s      0   0:00 sshd: jayven [priv]
    3218  3220  3218  3218 ?           -1 S<    1000   0:01 sshd: jayven@pts/0
    3220  3221  3221  3221 pts/0     3317 S<s   1000   0:00 -bash
    3221  3317  3317  3221 pts/0     3317 S<+   1000   0:00 vim io.py
       ......

   debian:/home/jayven# lsof -p 3218 
   COMMAND  PID USER   FD   TYPE     DEVICE    SIZE   NODE NAME
       ......
   sshd    3218 root    0u   CHR        1,3            563 /dev/null
   sshd    3218 root    1u   CHR        1,3            563 /dev/null
   sshd    3218 root    2u   CHR        1,3            563 /dev/null
   sshd    3218 root    3r  IPv4       7140            TCP 11.0.0.129:32201->192.168.23.70:2851 (ESTABLISHED)
   sshd    3218 root    4w  unix 0xde250c80           7167 socket
   sshd    3218 root    5u   CHR        5,2            958 /dev/ptmx
   sshd    3218 root    6u  unix 0xdf7f9e40           7169 socket
       ......

   debian:/home/jayven# lsof -p 3220
   COMMAND  PID   USER   FD   TYPE     DEVICE    SIZE   NODE NAME
       ......
   sshd    3220 jayven    3u  IPv4       7140            TCP 11.0.0.129:32201->192.168.23.70:2851 (ESTABLISHED)
   sshd    3220 jayven    4u  unix 0xde250c80           7167 socket
   sshd    3220 jayven    5u  unix 0xde250900           7168 socket
   sshd    3220 jayven    6r  FIFO        0,6           7172 pipe
   sshd    3220 jayven    7w  FIFO        0,6           7172 pipe
   sshd    3220 jayven    8u   CHR        5,2            958 /dev/ptmx
   sshd    3220 jayven   10u  unix 0xde250200           7178 /tmp/ssh-fKYAki3220/agent.3220
   sshd    3220 jayven   11u   CHR        5,2            958 /dev/ptmx
   sshd    3220 jayven   12u   CHR        5,2            958 /dev/ptmx
       ......

   debian:/home/jayven# lsof -p 3221
   COMMAND  PID   USER   FD   TYPE DEVICE    SIZE   NODE NAME
       ......
   bash    3221 jayven    0u   CHR  136,0              2 /dev/pts/0
   bash    3221 jayven    1u   CHR  136,0              2 /dev/pts/0
   bash    3221 jayven    2u   CHR  136,0              2 /dev/pts/0
       ......

   debian:/home/jayven# lsof -p 3317
   COMMAND  PID   USER   FD   TYPE DEVICE    SIZE   NODE NAME
       ......
   vim     3317 jayven    0u   CHR  136,0              2 /dev/pts/0
   vim     3317 jayven    1u   CHR  136,0              2 /dev/pts/0
   vim     3317 jayven    2u   CHR  136,0              2 /dev/pts/0
   vim     3317 jayven    4u   REG    8,1   12288 408855 /home/jayven/.io.py.swp
       ......

```

  * 可看出其流程大致如下:

```c

   /*       bash                     sshd
     __________________      ___________________                  ______________
    |                  |    |                   |    network     |              |
    | open(/dev/pts/0) | -> | open("/dev/ptmx") | ----...----->  | Your monitor |
    |                  |    |                   |                |              |
     ------------------      -------------------                  --------------
   */

```

  * 还可以这样

```bash

   jayven@debian:~$ w -h
   jayven   pts/0    192.168.23.70    23:09    0.00s  0.44s  0.00s w -h
   jayven@debian:~$ echo "something" > /dev/pts/0 
   something

```

### 未完待续 ###
...

### 参考 ###

  * <Advanced Programming in the Unix Environment> APUE 自然要看d
  * http://tldp.org/HOWTO/Text-Terminal-HOWTO.html 这个介绍写得很好
