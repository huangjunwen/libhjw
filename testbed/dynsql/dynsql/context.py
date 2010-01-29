from base import *

__all__ = ['Context']

class Context(object):

    def __init__(self, d, full_eval=True, none_as_nil=False):
        self.r = d                              
        self.w = {}                     # using another to avoid writing to the passed in dict
        self.full_eval = full_eval
        if full_eval:
            self.default = Nil
        else:
            self.default = Unknown

        if none_as_nil:
            def res_filter(x):
                if x is None:
                    return Nil
                return x
            self.res_filter = res_filter
        else:
            self.res_filter = lambda x: x

    def __getitem__(self, k):
        if self.w.has_key(k):
            res = self.w[k]
        else:
            res = self.r.get(k, self.default)       
        return self.res_filter(res)

    def __setitem__(self, k, v):
        self.w[k] = v
