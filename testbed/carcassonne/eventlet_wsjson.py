import re
try:
    from json import dumps, loads
except ImportError:
    from simplejson import dumps, loads

import eventlet

# ref: https://groups.google.com/group/json-rpc/web/json-rpc-1-2-proposal?pli=1

class JsonRPCErr(Exception):
    def __call__(self):
        ret = {'code': self.code, 'message': self.message}
        if self.args:
            ret['data'] = self.args[0]
        return ret;

class JsonRPCClientErr(JsonRPCErr):
    pass

class JsonRPCServerErr(JsonRPCErr):
    pass

class ParseErr(JsonRPCClientErr):
    code = -32700
    message = "Parse error."


class InvalidReq(JsonRPCClientErr):
    code = -32600
    message = "Invalid Request."


class MethodNotFound(JsonRPCClientErr):
    code = -32601
    message = "Method not found."


class InvalidParams(JsonRPCClientErr):
    code = -32602
    message = "Invalid params."

class InternalError(JsonRPCServerErr):
    code = -32603
    message = "Internal error."


class WSJsonRPCHandler(object):
    method_re = re.compile(r"^[_A-z]\w*$").match

    DEBUG = False

    def __init__(self, ws):
        self._ws = ws

    def _writer_func(self, queue):
        while True:
            pass

    def __call__(self):
        self._write_queue = q = eventlet.queue.Queue()
        self._writer = eventlet.greenthread.spawn(self._writer_func, q)

        while True:
            frame = self._ws.wait()
            if frame is None:
                break

            try:
                call_id, res = self._handle_frame(frame)
            except JsonRPCClientErr, e:
                self.close()
                break

            if call_id is not None:

    
    def _handle_frame(self, frame)
        call_id = None
        try:
            acall = loads(frame)
            if type(acall) is not dict:
                raise ValueError()
        except ValueError:
            return call_id, ParseErr()
        
        try:
            # get call id first (more friendly to the client)
            call_id = acall.get('id', None)                 
            version = acall['jsonrpc']
            method = acall['method']
            params = acall.get('params', [])
        except KeyError:
            return call_id, InvalidReq()
        
        if version != '2.0':
            return call_id, InvalidReq()
        if type(method) not in (unicode, str) or not self.method_re(method):
            return call_id, InvalidReq()

        # TODO need log
        method = self.dispatch(method)
        if method is None:
            return call_id, MethodNotFound()
            
        # run (no dict params support here)
        if type(params) is not list:
            return call_id, InvalidParams()

        try:
            return call_id, method(*params)
        except Exception, e:
            # TODO need log
            return call_id, InternalError(e)

    def respAny(self, val, call_id):
        if not isinstance(val, failure.Failure) and not isinstance(val, Exception):
            return self.respResult(val, call_id)

        if isinstance(val, failure.Failure):
            log.err(val)
        else:
            from traceback import format_exc
            log.msg(format_exc())

        if isinstance(val, failure.Failure):
            val = val.value
        if not isinstance(val, JsonRPCErr):         # convert to JsonRPCErr
            val = InternalError()
        return self.respErr(val(), call_id)
        
    def respResult(self, res, call_id):
        self.transport.write(dumps({'jsonrpc': '2.0', 'result': res, 'id': call_id}))
            
            
    def respErr(self, err, call_id):
        self.transport.write(dumps({'jsonrpc': '2.0', 'error': err, 'id': call_id}))
            
    def dispatch(self, method):
        return getattr(self, "do_" + method, None)

    def notify(self, method, params):
        """
        'Call' to the client, but without response
        """
        self.transport.write(dumps({'jsonrpc': '2.0', 'method': method, 'params': params}))

    def connectionLost(self, reason):
        """
        Override it if you want to handle connection lost
        """

    def close(self):
        self.transport.loseConnection()


