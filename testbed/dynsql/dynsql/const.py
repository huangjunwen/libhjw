
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
