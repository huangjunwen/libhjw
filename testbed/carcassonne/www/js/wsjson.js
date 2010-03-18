// vim::nu

/* json-rpc through websocket transport */
/* required: 
 *     mootool-core.js
 * provide:
 *     Class WSJson
 */

var WSJsonSt = { notOpened: 0, 
    opened: 1, 
    closed: -1 
};

var WSJson = (function() {

var idCounter = 0;
function uniqID() {
    return idCounter++;                                 // XXX may wrap round
}

return new Class({

    Implements: [Events, Options],

    options: {
        /*
         * onOpen: function()
         * onClose: function()
         * onError: function()
         */
        callbacks: {}
    },

    initialize: function(opt) {
        this.setOptions(opt);

        // attr
        this.status = WSJsonSt.notOpened;                               // 0 not opened; 1 opened; -1 closed
        this.outstandCalls = new Hash();                                // id -> callback
        this.transport = null;
    },
    connect: function(url) {
        if (this.transport)
            return;

        var inst = this;
        this.transport = new WebSocket(url);
        this.transport.onopen = function() {
            inst.status = WSJsonSt.opened;
            inst.fireEvent('open');
        };
        this.transport.onclose = function() {
            if (inst.status == WSJsonSt.notOpened)
                inst.fireEvent('error');
            else
                inst.fireEvent('close');
            inst.status = WSJsonSt.closed;
        },
        this.transport.onmessage = function(ev) {
            try {
                var res = JSON.decode(ev.data);
                if (res.jsonrpc != '2.0')
                    throw "bad server protocol";

                if (res.method) {                                       // a notify from server
                    var method = inst.options.callbacks[res.method];
                    if (!method)
                        throw "can't find method: {method}".substitute(res);
                    method.apply(inst, res.params);
                } else {                                                // response
                    var cb = inst.outstandCalls.get(res.id);
                    if (!cb)
                        throw "no such call id: {id}".substitute(res);
                    inst.outstandCalls.erase(res.id);
                    if (res.error)
                        throw res.error.message;
                    cb(res.result);                                     // callback prototype
                }
            } catch (e) {
                alert(e);
                inst.close();
                return;
            }
        };
    },
    getStatus: function() {
        return this.status;
    },
    close: function() {
        if (this.status == WSJsonSt.opened)
            this.transport.close();
    },
    call: function(method, params, cb) {
        var id = uniqID();
        this.outstandCalls.set(id, cb || $empty);
        var c = JSON.encode({'jsonrpc': '2.0', 'method': method, 
            'params': params, 
            'id': id
        });

        if (this.status != WSJsonSt.opened)
            throw "transport not opened";

        this.transport.send(c);
        return id;
    }
});

})();


