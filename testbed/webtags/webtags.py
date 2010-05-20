#!/usr/bin/env python
# -*- coding=utf-8 -*-

import web

urls = (
   r'/([\w\-]+(?:\.\w+)?)$', 'ShowCode',
   r'/static/([\w\-]+(?:\.\w+)?)$', 'StaticFile'
)

app = web.application(urls, globals())

# for template render
common_globals = {
    'str': str,
}

render = web.template.render('template/', globals=common_globals)

class CacheFileMeta(type):
    
    def __new__(mcls, name, base, attr):
        attr['_cache'] = {}

        def enable_cache(f):
            def ret(self, fname):
                if fname not in self._cache:
                    self._cache[fname] = f(self, fname)
                return self._cache[fname]
            return ret
        
        if 'GET' in attr:
            attr['GET'] = enable_cache(attr['GET'])
        if 'POST' in attr:
            attr['POST'] = enable_cache(attr['POST'])
            
        return type.__new__(mcls, name, base, attr)


class ShowCode(object):

    __metaclass__ = CacheFileMeta

    def GET(self, fname):
        try:
            flines = open(fname).readlines()
        except IOError:
            raise web.notfound("Source file not found")

        return render.show_code(fname, flines)


class StaticFile(object):

    __metaclass__ = CacheFileMeta

    def GET(self, fname):
        try:
            return open('static/' + fname).read()
        except IOError:
            raise web.notfound("File not found")
        

if __name__ == "__main__": 
    app.run()
