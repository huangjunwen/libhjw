#!/usr/bin/env python

import os, sys, struct
import termios, tty
from fcntl import ioctl


def interactive_shell(chan):
    # copy from demo/interactive.py, only support posix os
    import select, socket
    
    oldtty = termios.tcgetattr(sys.stdin)
    try:
        tty.setraw(sys.stdin.fileno())
        tty.setcbreak(sys.stdin.fileno())
        chan.settimeout(0.0)

        while True:
            try:
                r, w, e = select.select([chan, sys.stdin], [], [])
            except select.error:
                continue
            except Exception, e:
                raise e

            if chan in r:
                try:
                    x = chan.recv(1024)
                    if len(x) == 0:
                        break
                    sys.stdout.write(x)
                    sys.stdout.flush()
                except socket.timeout:
                    pass
            if sys.stdin in r:
                #x = sys.stdin.read(1)
                x = os.read(sys.stdin.fileno(), 1)
                if len(x) == 0:
                    break
                chan.send(x)

    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, oldtty)


winsz_fmt = "HHHH"
winsz_arg = " "*struct.calcsize(winsz_fmt)


def term_winsz():
    """Return terminal window size (height, width)"""
    if not sys.stdin.isatty():
        raise type("NotConnectToTTYDevice", (Exception,), {})()
    return struct.unpack(winsz_fmt, ioctl(sys.stdin, termios.TIOCGWINSZ, winsz_arg))[:2]


def term_type():
    """Return terminal type"""
    return os.environ.get("TERM", "linux")


def run_shell(client):
    """Pass in a connected SSHClient and run a shell"""
    import signal

    # get current terminal's settings
    height, width = term_winsz()
    tt = term_type()

    # remember current signal handler
    chan = None
    old_handler = signal.getsignal(signal.SIGWINCH)
    def on_win_resize(signum, frame):
        if chan is not None:
            height, width = term_winsz()
            chan.resize_pty(width=width, height=height)
    signal.signal(signal.SIGWINCH, on_win_resize)

    try:
        # invoke shell
        chan = client.invoke_shell(tt, width=width, height=height)
        interactive_shell(chan)
    finally:
        chan.close()
        signal.signal(signal.SIGWINCH, old_handler)


def test(host, port, user):
    import paramiko
    agent = paramiko.Agent()
    if not agent.keys:
        raise type("NoKeySupply", (Exception,), {})()
    pkey = agent.keys[0]

    client = paramiko.SSHClient()
    client.load_system_host_keys()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(host, port, user, pkey=pkey)

    try:
        run_shell(client)
    finally:
        client.close()

if __name__ == '__main__':
    host, port, user = sys.argv[1:]
    port = int(port)
    test(host, port, user)
