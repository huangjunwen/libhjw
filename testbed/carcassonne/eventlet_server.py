import sys
import re
from os.path import normpath, abspath, isfile
import eventlet
from eventlet import wsgi, websocket

########################
# static files
########################

_legal_part = re.compile(r'^[\w\.\-]+$').match

def _legal_path(path):
    parts = path.split('/')
    for part in parts:
        if part and not _legal_part(part):
            return False
    return True

_mime_map = {
    "html": "text/html",
    "js": "application/x-javascript",
    "css": "text/css",
    "png": "image/png",
}

def _content_type(fname=""):
    try:
        _, postfix = fname.rsplit(".", 1)
    except:
        postfix = ""
    return ('content-type', _mime_map.get(postfix, "text/plain"))
    
def _200(start_response, fname, content):
    start_response('200 OK', [_content_type(fname),])
    return content

def _404(start_response):
    start_response('404 Not Found', [_content_type(),])
    return '404 Not Found'

def _500(start_response, err):
    start_response('404 Not Found', [_content_type(),])
    return err

_static_caches = {}

def handle_static(root_dir, index, environ, start_response):
    path = environ['PATH_INFO']
    # check path legal
    if not _legal_path(path):
        return _404(start_response)

    # get real path
    if path == '/':
        path = index
    real_path = abspath(normpath('/'.join([root_dir, path])))

    # check file in cache
    if real_path not in _static_caches:

        # check file on disk
        if not isfile(real_path):
            return _404(start_response)
        
        # try read and put into cache
        try:
            _static_caches[real_path] = open(real_path).read()
        except:
            return _500(start_response, "Permission denied")

    return _200(start_response, real_path, _static_caches[real_path])
    
########################
# web sockets
########################

@websocket.WebSocketWSGI
def handle_web_socket(ws):
    pass

########################
# dispatch
########################

def dispatch(environ, start_response):
    path = environ['PATH_INFO']

    if path == '/ws/carcassonne':
        return handle_web_socket(environ, start_response)

    return handle_static('www', 'carcassonne.html', environ, start_response)


if __name__ == '__main__':

    addr = '0.0.0.0'
    try:
        port = int(sys.argv[1])
    except IndexError, ValueError:
        port = 8081

    wsgi.server(eventlet.listen((addr, port)), dispatch)



