
class Nil(object):
    def __nonzero__(self):
        return False
Nil = Nil()

class NotNil(object):
    def __nonzero__(self):
        return True
NotNil = NotNil()

class Unknown(object):
    def __nonzero__(self):
        raise ValueError("Can't eval on unkown object")             # a guard
Unknown = Unknown()

class Extendable(type):
    
    def __new__(mcs, name, base, attr): 
        if name != '__extend__':
            return type.__new__(mcs, name, base, attr)
        for b in base:
            for n in attr:
                setattr(b, n, attr[n])
