# -*- encoding=utf-8 -*-

class EventObject(object):
    
    def __init__(self, ev_type, src, args):
        self.ev_type = ev_type
        self.src = src
        self.args = args

    def __repr__(self):
        return "<%s event object>" % self.ev_type
    
    def __getattr__(self, name):
        return self.args.get(name, None)


class EventSrc(object):

    def __init__(self):
        self.eventListeners = {}            # ev_type -> [listeners]
        self._pendingCalls = []             # avoid modifing the list when traversing
        self._firingEv = False

    def _callPendings(self):
        for name, args, kw in self._pendingCalls:
            getattr(self, name)(*args, **kw)
        self._pendingCalls = []

    def addEvListener(self, ev_type, listener):
        if self._firingEv:
            self._pendingCalls.append(('addEvListener', (ev_type, listener), {}))
            return

        listeners = self.eventListeners.setdefault(ev_type, [])
        listeners.append(listener)

    def rmEvListener(self, ev_type=None, listener=None):
        if self._firingEv:
            self._pendingCalls.append(('rmEvListener', (ev_type, listener), {}))
            return

        if ev_type is None:
            self.eventListeners = {}
            return
        if listener is None:
            self.eventListeners[ev_type] = []
            return
        try:
            self.eventListeners.get(ev_type, []).remove(listener)
        except ValueError:
            pass
    
    def addOnceEvListener(self, ev_type, listener):
        def once(ev):
            listener(ev)
            self.rmEvListener(ev_type, once)
        self.addEvListener(ev_type, once)

    def fireEv(self, ev_type, **args):
        ev = EventObject(ev_type, self, args)
        self._firingEv = True
        for l in self.eventListeners.get(ev_type, []):
            l(ev)                                       # listener prototype
        self._firingEv = False
        self._callPendings()


if __name__ == '__main__':
    def La(ev):
        print "La handling %r" % ev

    def Lb(ev):
        print "Lb handling %r" % ev

    def Lc(ev):
        print "Lc handling %r" % ev
        ev.src.addOnceEvListener(ev.ev_type, Lc)

    def Ld(ev):
        print "Ld handling %r" % ev
        ev.src.rmEvListener()

    s = EventSrc()
    s.addEvListener('bomb', La)
    s.addOnceEvListener('bomb', Lb)
    s.addOnceEvListener('bomb', Lc)
    s.fireEv('bomb')
    s.fireEv('bomb')
    s.addOnceEvListener('bomb', Ld)
    s.addOnceEvListener('bomb', Lb)
    s.fireEv('bomb')
    s.fireEv('bomb')


