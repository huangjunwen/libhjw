import web

urls = (
   r'/([\w\-\.]*)$', 'show_code',
   r'/static/([\w\-\.]+)$', 'static'
)

app = web.application(urls, globals())

class show_code:
    def GET(self, filename):
        if not filename:
            raise web.notfound("Plz specified the source file name")
        globals = {'str': str}
        render = web.template.render('template/', globals=globals)
        try:
            return render.show_code(filename, open(filename).readlines())
        except IOError:
            raise web.notfound("Source file not found")

class static:
    cache = {}
    def GET(self, filename):
        try:
            return self.cache[filename]
        except KeyError:
            pass

        try:
            content = open('static/' + filename).read()
        except IOError:
            raise web.notfound("File not found")
        
        self.cache[filename] = content
        return content
        

if __name__ == "__main__": 
    app.run()
