// vim::nu

/* carcassonne game logic */
/* required: 
 *     mootool-core.js
 *     res.js
 */

/* Organize all uniq identified object as a tree
 * objs will be deleted when its parent is deleted
 */
var UniqObj = (function() {
    var allObjs = new Hash();

    var ret = new Class({
        initialize: function(id) {
            var obj = allObjs.get(id);
            if (obj)
                throw "obj " + id + " already created";

            this.uniqID = id;
            allObjs.set(id, this);
        },
        toID: function() {
            return this.uniqID;
        },
        finalize: function() {               // finalize will delete all child objs as well
            ret.finalize(this.toID());
        }
    });
    $extend(ret, {
        fromID: function(id) {
            return allObjs.get(id);
        },
        finalize: function(id) {
            var obj = allObjs.get(id);
            if (!obj)
                return;
             
            // remove from global
            allObjs.erase(id);
            delete obj;
        },
        cleanAll: function() {
            allObjs = new Hash();
        }
    });
    return ret;
})();

var Player = new Class({

    Extends: UniqObj,

    initialize: function(id, nickname) {
        this.parent(id);
        this.nickname = nickname;
        this.colorID = -1;                          // manipulate by score board
    },
    asSelf: function() {
        $extend(Player, {'self': this});
    }
});

var Meeple = (function() {

    var halfMeepleImgSize = (meepleImgSize/2).toInt();

    return new Class({

        Extends: UniqObj,

        Implements: [Events, Options],

        options: {
            /*
             * onShow: function(meeple, pos)
             * onHide: function(meeple)
             * onClick: function(meeple)
             * */
        },

        initialize: function(id, colorID, opt) {
            this.parent(id);
            this.setOptions(opt);

            // attr
            this.colorID = colorID;
            this.hided = true;

            // DOM
            this.el = new Element('div', {
                "styles": {
                    "padding": "0px",
                    "width": meepleImgSize + "px",
                    "height": meepleImgSize + "px",
                    "background-position": "{x}px 0px".substitute({x: -colorID * meepleImgSize}),
                    "background-image": "url('" + meepleImgUrl + "')",
                    "background-repeat": "no-repeat",
                    "zIndex": 1000,
                    "display": "none",
                    "position": "absolute",
                    "cursor": "pointer"
                }
            });
            var inst = this;
            this.el.addEvent('click', function(ev) {
                inst.fireEvent('click', [inst]);
                return false;                                   // do not bubble up
            });
        },
        toElement: function() {
            return this.el;
        },
        hide: function() {
            if (this.hided)
                return;
            this.hided = true;
            this.el.setStyle('display', 'none');
            this.fireEvent('hide', [this]);
        },
        show: function(pos) {
            this.hide();
            this.hided = false;
            pos = {x: pos.x - halfMeepleImgSize, y: pos.y - halfMeepleImgSize};
            this.el.setPosition(pos);
            this.el.setStyle('display', 'block');
            this.fireEvent('show', [this, pos]);
        },
        stop: function() {
            this.removeEvents();
        }
    });
})();


var Tile = (function() {

    // private methods

    var coordConvs = [function(x,y){return [x,y];}, function(x,y){return [tileImgSize-y,x];}, 
        function(x,y){return [tileImgSize-x,tileImgSize-y];}, 
        function(x,y){return [y,tileImgSize-x];}
    ];

    var mapName = function(uniqID, rotation) {
        return 'TM' + uniqID + '_' + rotation;
    };

    function setTargetPos(pos) {
        if (this.frozed)
            return;
        this.targetPos = pos;
        this.shadow.setPosition(pos);
    }

    function morphToTargetPos() {
        if (this.frozed)
            return;
        var pos = this.targetPos;
        this.el.morph({left: pos.x, top: pos.y});
        return this.el.get('morph');
    }

    return new Class({

        Extends: UniqObj,

        Implements: [Events, Options],

        options: {
            /*
             * onPicked: function(tile)
             * onDrag: function(tile, ev)
             * onPut: function(tile, pos)
             * onRotate: function(tile)
             * onTerraClick: function(tile, terra, terraType, ev)
             * */
        },

        initialize: function(id, tileIdx, opt) {
            this.parent(id);
            this.setOptions(opt);

            // alias
            var inst = this;

            // attr
            this.tileIdx = tileIdx;
            this.rotation = -1;
            this.fit = null;
            this.frozed = false;
            this.notDragged = true;                                 // XXX a tag to tell between click and drag

            // DOM: create container div and the img ( for image map )
            this.el = new Element('div');
            this.el.store('tile', this);
            this.el.set('morph', {duration: 'short', transition: Fx.Transitions.Sine.easeOut});
            this.img = new Element('img', {
                "border": "0",
                "src": tileTransparentUrl,                          // FF needs a src, otherwise image map will not work
                "styles": {
                    "padding": "0px",
                    "display": "block",
                    "background-image": "url('" + tilesImgUrl + "')",
                    "background-repeat": "no-repeat",
                    "width": tileImgSize + "px",
                    "height": tileImgSize + "px"
                }
            });
            this.img.inject(this.el);

            // image maps
            $each(coordConvs, function(coordConv, r) {
                var map = new Element('map', {'name': mapName(inst.toID(), r)});
                $each(tilesMap[inst.tileIdx], function(a) {
                    // calculate coords
                    var coords = [];
                    $each(a.coords, function(c) {
                        coords.push(coordConv(c[0], c[1]).join(','));
                    });
                    coords = coords.join(',');

                    // add element to the map
                    var areaEl = new Element('area', {shape: a.shape, terra: a.terra, alt: a.title,
                        title: a.title,
                        nohref: true,
                        coords: coords
                    });
                    areaEl.addEvent('click', function(ev) {
                        if (inst.notDragged) 
                            inst.fireEvent('terraclick', [inst, a.terra, a.title, ev]);     // XXX fire click only when no drag
                        else
                            inst.notDragged = true;                                         // clear the tag
                    });
                    areaEl.inject(map);
                });
                map.inject(inst.el);
            });

            // shadow (shadow is used for positioning)
            this.shadow = this.img.clone(false);
            this.shadow.setStyles({
                'display': 'block',
                'position': 'absolute',
                'opacity': 0.5
            });

            // init rotation
            this.rotate();
        },
        finalize: function() {
            if (this.el)
                this.el.dispose();
            if (this.shadow)
                this.shadow.dispose();
            this.freeze();
            this.parent();
        },
        // DOMs
        toElement: function() {                         // ref: http://n2.nabble.com/Extends-Element-in-1-2-td796845.html
            return this.el;
        },
        getAreas: function() {                          // find areas in all rotation
            return this.el.getElements('map area');
        },
        // data
        getBound: function(d) {
            return tilesBounds[this.tileIdx][(4 + d - this.rotation)%4];
        },
        // styles
        setFit: function() {
            this.img.setProperty('src', tileTransparentUrl);
            this.shadow.setProperty('src', tileTransparentUrl);
            this.fit = true;
        },
        setUnFit: function() {
            this.img.setProperty('src', tileTransparentRedUrl);
            this.shadow.setProperty('src', tileTransparentRedUrl);
            this.fit = false;
        },
        // actions
        injectAt: function(cont, pos) {                 // this should be called before any other actions
            if (this.frozed)
                return;
            this.shadow.inject(cont);
            this.setTargetPos(pos);
            this.el.inject(cont).setPosition(pos);
            this.fireEvent('put', [this, this.targetPos]);
        },
        setTargetPos: setTargetPos,
        makeDraggable: function() {
            if (this.dragger) {
                this.dragger.attach();
                return;
            }
            var inst = this;
            this.dragger = new Drag.Move($(inst), {
                onStart: function(el) {
                    inst.fireEvent('picked', [inst]);
                    inst.notDragged = false;            // XXX
                },
                onDrag: function(el, ev) {
                    inst.fireEvent('drag', [inst, ev]);
                },
                onComplete: function(el) {
                    inst.fireEvent('put', [inst, inst.targetPos]);
                    morphToTargetPos.call(inst);
                }
            });
        },
        stopDraggable: function() {
            if (this.dragger)
                this.dragger.detach();
        },
        moveTo: function(pos) {
            if (this.frozed)
                return;
            this.setTargetPos(pos);
            this.fireEvent('picked', [this]);
            this.fireEvent('put', [this, this.targetPos]);
            return morphToTargetPos.call(this);
        },
        rotate: function(r) {
            if (this.frozed)
                return;
            r = r || 1;
            // update rotation
            this.rotation = (this.rotation + r) % 4;

            // update background image
            var bgPos = "{x}px {y}px".substitute({ x: -this.tileIdx * tileImgSize, y: -this.rotation * tileImgSize })
            this.img.setStyle('background-position', bgPos);
            this.shadow.setStyle('background-position', bgPos);

            // update map
            this.img.setProperty('usemap', '#' + mapName(this.toID(), this.rotation));
            this.fireEvent('rotate', this);
        },
        freeze: function() {        
            this.stopDraggable();
            this.frozed = true;
        },
        unfreeze: function() {        
            this.frozed = false;
            this.makeDraggable();
        },
        stop: function() {
            /* 
             * stop this tile, remove all events on it
             * */
            if (!this.shadow)
                return;
            this.freeze();
            this.removeEvents();
            var currMapName = mapName(this.toID(), this.rotation);
            $each(this.el.getElements("map[name!=" + currMapName + "]"), function(m) { 
                m.dispose(); 
            });
            this.shadow.dispose();
            this.shadow = null;
        }
    });
})();


var Board = (function() {

    var Grid = new Class({                                      // 2D dict, values stored in Grid can has a toID method
        initialize: function() {
            this.d = new Hash();
            this.rd = new Hash();
        },
        get: function(gX, gY) {
            var r = this.d.get(gX);
            if (!r)
                return null;
            return r.get(gY);
        },
        find: function(v) {
            if (!v || !v.toID)
                return null;
            return this.rd.get(v.toID());
        },
        set: function(gX, gY, v) {
            if (!this.d.has(gX))
                this.d.set(gX, new Hash());
            this.d.get(gX).set(gY, v);

            if (v && v.toID)
                this.rd.set(v.toID(), {'gX': gX, 'gY': gY});
        },
        erase: function(gX, gY) {
            var r = this.d.get(gX);
            if (!r)
                return;
            var v = r.get(gY);
            r.erase(gY);
            if (v && v.toID)
                this.rd.erase(v.toID());
        },
        each: function(f) {
            this.d.each(function(v1) {
                v1.each(function(v) {
                    f(v);
                });
            });
        },
        apply: function(gX, gY, arg, f) {
            var r = f(this.get(gX, gY), arg);
            if (!$defined(r))
                return;
            this.set(gX, gY, r)
        },
        applyNeighbor: function(gX, gY, f) {                    // for each of the 4 neighbors
            this.apply(gX, gY + 1, 0, f);                       // up
            this.apply(gX + 1, gY, 1, f);                       // right
            this.apply(gX, gY - 1, 2, f);                       // down
            this.apply(gX - 1, gY, 3, f);                       // left
        }
    });

    var boundOpposite = [2, 3, 0, 1];

    return new Class({

        Extends: UniqObj,

        initialize: function(id) {
            this.parent(id);

            // attr
            this.neighborCnt = new Grid();                      // (x, y) -> neighbor count
            this.tiles = new Grid();                            // (x, y) -> tile
            this.tilesCnt = 0;

            // DOM
            this.el = new Element('div', {
                'styles': {
                    'background-color': '#e0e0e0',
                    'border': 'none',                           // no border for caculating coord simpler
                    'position': 'absolute',
                    'width': tileImgSize + "px",
                    'height': tileImgSize + "px"
                }
            });
        },
        finalize: function() {
            this.tiles.each(function(t) {
                t.finalize();
            });
            if (this.el)
                this.el.dispose();
            this.parent();
        },
        toElement: function() {
            return this.el;
        },
        getNeighborCnt: function(c) {
            return this.neighborCnt.get(c.gX, c.gY) || 0;
        },
        getGridCoord: function(c, relative) {                                      // c is the coord relative to the argument 'relative'
            var pos = this.el.getPosition(relative);
            var x = c.x - pos.x, y = c.y - pos.y;
            var modX = x%tileImgSize, modY = y%tileImgSize;
            if (modX < 0)
                modX = tileImgSize + modX;
            if (modY < 0)
                modY = tileImgSize + modY;
            x -= modX;
            y -= modY;
            gX = (x/tileImgSize).toInt();
            gY = - (y/tileImgSize).toInt();
            return {'x': x, 'y': y, 'gX': gX, 'gY': gY};
        },
        occupied: function(c) {
            return this.tiles.get(c.gX, c.gY) ? true : false;
        },
        checkBounds: function(c, tile) {
            if (this.tilesCnt == 1 && !c.gX && !c.gY)
                return true;

            var cnt = 0;
            var fit = true;
            this.tiles.applyNeighbor(c.gX, c.gY, function(t, d) {
                if (!t)
                    return;
                ++cnt;
                if (tile.getBound(d) != t.getBound(boundOpposite[d]))
                    fit = false;
            });

            return cnt != 0 && fit;
        },
        findTile: function(tile) {
            var c;
            if (!(c = this.tiles.find(tile)))
                return null;
            c.x = c.gX*tileImgSize;
            c.y = c.gY*tileImgSize;
            return c;
        },
        createTile: function(id, tileIdx, c) {                                      // c should be a grid coord
            if (this.occupied(c))
                return null;

            var board = this;
            var tile = new Tile(id, tileIdx, {
                onPicked: function(t) {
                    var c = board.findTile(t);
                    var gX = c.gX;
                    var gY = c.gY;
                    board.neighborCnt.applyNeighbor(gX, gY, function(v, d) {        // all neighbors -1 count
                        return (v || 0) - 1;
                    });
                    board.tiles.erase(gX, gY);
                    --board.tilesCnt;
                    t.setFit();
                },
                onDrag: function(t, ev) {
                    var c = board.getGridCoord(ev.page);
                    if (board.occupied(c))
                        return;
                    t.setTargetPos(c);
                },
                onPut: function(t, c) {
                    var gX = c.gX;
                    var gY = c.gY;
                    board.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors +1 count
                        return (v || 0) + 1;
                    });
                    board.tiles.set(gX, gY, tile);
                    ++board.tilesCnt;
                    
                    // check fit
                    board.checkBounds(c, t) ? t.setFit() : t.setUnFit();
                },
                onRotate: function(t) {
                    var c = board.findTile(t);
                    if (!c)
                        return;
                    board.checkBounds(c, t) ? t.setFit() : t.setUnFit();
                }
            });

            tile.injectAt(this, c);
            return tile;
        }
    }); 
})();


var Panel = new Class({

    Implements: [Events, Options],

    options: {},

    initialize: function(opt) {
        this.setOptions(opt);
        this.reset();
    },
    show: function() {
        $(this).setStyle("display", "block");
    },
    hide: function() {
        $(this).setStyle("display", "none");
    },
    reset: function() {
        throw "Each panel should has a reset method to restore the init status";
    }
});

var GamePanel = (function() {
    
    function scoreBoard(colorID) { return $('scoreBoard' + colorID); }
    function playerName(colorID) { return $('playerName' + colorID); }
    function meepleCnt(colorID) { return $('meepleCnt' + colorID); }
    function score(colorID) { return $('score' + colorID); }
    function ready(colorID) { return $('ready' + colorID); }
    function readySt(colorID) { return $('readySt' + colorID); }

    return new Class({

        Extends: Panel,

        options: {
            /*
             * onReady: function()
             * onMvCentClick: function()
             * onTurnEndClick: function()
             * */
        },
        initialize: function(opt) { 
            this.el = $('gamePanel');
            this.players = [null, null, null, null, null];

            var inst = this;
            $('mvCenter').addEvent('click', function(ev) {
                inst.fireEvent('mvcentclick', []);
            });
            $('turnEnd').addEvent('click', function(ev) {
                inst.fireEvent('turnendclick', []);
            });

            this.parent();
        },
        toElement: function() {
            return this.el;
        },
        reset: function() {
            var inst = this;
            $each(this.players, function(p) {
                if (!p)
                    return;
                inst.unselectColor(p);
            });
            this.resetAllScoreBoards();
        },
        resetScoreBoard: function(colorID) {
            meepleCnt(colorID).set('text', 7);
            score(colorID).set('text', 0);
            readySt(colorID).set('html', 'No');
        },
        resetAllScoreBoards: function() {
            for (var i = 0; i < 5; ++i)
                this.resetScoreBoard(i);
        },
        becomeReady: function(player) {
            readySt(player.colorID).set('html', '<font color="green">Yes</font>');
        },
        makeReadyClickable: function() {
            var panel = this;
            var player = Player.self;
            if (player.colorID < 0)
                return false;
            ready(player.colorID).addEvent('click', function(ev) {
                ready(player.colorID).removeEvents('click').setStyles({"cursor": "default", "text-decoration": "none"});
                panel.becomeReady(player);
                panel.fireEvent('ready');
            }).setStyles({"cursor": "pointer", "text-decoration": "underline"});
            return true;
        },
        unselectColor: function(player) {                               // can only called by server
            if (player.colorID < 0) 
                return;

            var colorID = player.colorID;
            player.colorID = -1;
            this.players[colorID] = null;

            // alter page content
            this.resetScoreBoard(colorID);
            scoreBoard(colorID).removeClass('scoreBoardSelected');      // XXX css
            playerName(colorID).set('text', '');
        },
        selectColor: function(player, colorID) {                        // can only called by server
            if (this.players[colorID])
                return;

            player.colorID = colorID;
            this.players[colorID] = player;

            // alter page content
            scoreBoard(colorID).addClass('scoreBoardSelected');
            playerName(colorID).set('text', player.nickname);
        }
    });
})();

var MsgPanel = (function() {
    return new Class({
        Extends: Panel,

        options: {
            /*
             * onChat: function(msg)
             * */
        },
        initialize: function() {
            var inst = this;
            this.el = $("msgPanel");
            $("msgForm").addEvent("submit", function(ev) {
                var msg = $("msg").getProperty("value");
                if (!msg) {
                    alert("请不要发送空信息");
                    return false;
                }
                inst.fireEvent('chat', [msg]);
                inst.chatMsg(msg);
                $("msg").setProperty("value", "");
                return false;
            });
            this.parent();
        },
        toElement: function() {
            return this.el;
        },
        reset: function() {
            $("msgHelp").set("text", "");
            $("msgHistory").set("html", "");
            $("msg").setProperty("value", "");
        },
        chatMsg: function(msg, player) {
            (new Element("p", {text: (player ? player.nickname : "您") + "说: " + msg
                })).inject($("msgHistory"));
            $("msg").set("text", "");
        },
        sysMsg: function(msg) {
            (new Element("p", {text: "系统消息: " + msg, styles: {
                    color: "red"
                }
            })).inject($("msgHistory"));
        }
    });
})();

var LoginPanel = (function() {
    return new Class({
        Extends: Panel,

        options: {
            //
            // onSubmit: function(nickname, gameID)
            //
        },

        initialize: function(opt) {
            this.el = $("loginPanel");
            var inst = this;
            $("loginForm").addEvent("submit", function(ev) {
                var nickname = $("nickname").getProperty("value");
                if (!nickname) {
                    inst.showInput("请输入您的昵称");
                    return false;
                }
                if (nickname.length > 10) {
                    inst.showInput("您的昵称过长");
                    return false;
                }
                var gameID = $("gameID").get("value").toInt();
                inst.showWait();
                inst.fireEvent("submit", [nickname, gameID]);
                return false;
            });

            this.parent();
        },
        toElement: function() {
            return this.el
        },
        reset: function() {
            $("nickname").setProperty("value", "");
            $("gameID").set("value", 0);
            this.showInput();
        },
        showInput: function(err) {
            $("loginErr").set("text", err || "");
            $("loginWait").setStyle("display", "none");
            $("loginFormCont").setStyle("display", "block");
        },
        showWait: function() {
            $("loginFormCont").setStyle("display", "none");
            $("loginWait").setStyle("display", "block");
        }
    });
})();

/*
Element.implement({
    mvToPageCenter: function(offsetX, offsetY) {
        offsetX = offsetX || 0;
        offsetY = offsetY || 0;
        var coord = window.getCoordinates();
        var elCoord = this.getCoordinates();
        this.setPosition({
            x: ((coord.width - elCoord.width)/2).toInt() + offsetX,
            y: ((coord.height - elCoord.height)/2).toInt() + offsetY
        });
    }
});
*/

function Carcassonne() {
    var transport, loginPanel, gamePanel, msgPanel, board;

    loginPanel = new LoginPanel();
    gamePanel = new GamePanel();
    msgPanel = new MsgPanel();

    function reset(err) {
        if (board)
            board.finalize();
        board = null;
        gamePanel.hide();
        gamePanel.reset();
        msgPanel.hide();
        msgPanel.reset();
        loginPanel.reset();
        loginPanel.showInput(err);
        loginPanel.show();
        UniqObj.cleanAll();
    }
    reset();

    /************************
     * RPCs called to server 
     ************************/

    // called when enter a nickname in login panel and submit
    function _join(nickname, gameID) {
        transport.call('join', [nickname, gameID], function(callID, res) {
            if (!res.ok) {                                          // XXX res {ok: true/false, msg: x, selfID: x, players: [...]}
                loginPanel.showInput(res.msg);
                return;
            }

            loginPanel.hide();

            gamePanel.show();
            // XXX res.players [{id: x, nickname: x, colorID: x, ready: true/false}, ...]
            $each(res.players, function(u) {                                              
                var p = new Player(u.id, u.nickname);
                if (u.id == res.selfID)
                    p.asSelf();
                gamePanel.selectColor(p, u.colorID);
                if (u.ready)
                    gamePanel.becomeReady(p);       
            });
            gamePanel.makeReadyClickable();

            msgPanel.show();
            msgPanel.sysMsg("您进入房间 " + gameID);
        });
    }
    loginPanel.addEvent('submit', function(nickname, gameID) {
        if (transport) {
            _join(nickname, gameID)
            return false;
        }
        createTransport();
        transport.addEvent('open', function() {
            _join(nickname, gameID);
        });
        return false;
    });

    // called when the player click ready
    function _ready() {
        transport.call('ready', [], function(callID, res) {
            if (!res.ok)                                            // XXX res {ok: true/false}
                throw "ready should return ok";
            msgPanel.sysMsg("您准备好了");
        });
    }
    gamePanel.addEvent('ready', _ready);

    // called when the player chat
    function _chat(msg) {
        transport.call('chat', [msg], function(callID, res) {
            if (!res.ok) {
                msgPanel.sysMsg('"' + msg + '" 没有发送成功');
            }
        });
    }
    msgPanel.addEvent('chat', _chat);

    /************************
     * RPCs called by server 
     ************************/

    // XXX
    var _callbacks = {
        join: function(id, nickname, colorID) {
            gamePanel.selectColor(new Player(id, nickname), colorID);
            msgPanel.sysMsg(nickname + " 进入房间");
        },
        leave: function(id) {
            var p = UniqObj.fromID(id);
            gamePanel.unselectColor(p);
            msgPanel.sysMsg(p.nickname + " 退出了房间");
        },
        ready: function(id) {
            var player = UniqObj.fromID(id);
            gamePanel.becomeReady(player);
            msgPanel.sysMsg(player.nickname + " 准备好了");
        },
        chat: function(id, msg) {
            // if (id == Player.self.toID())                                           // filter out
            //    return false;
            msgPanel.chatMsg(msg, UniqObj.fromID(id));
        }
    };

    /************************
     * The transport
     ************************/

    function createTransport() {
        transport = new WSJson({
            onError: function() {
                loginPanel.showInput("无法连接.");
            },
            onClose: function(){
                transport = null;
                reset("服务连接中断.");
            },
            callbacks: _callbacks
        });
        transport.connect("ws://{host}/ws/carcassonne".substitute(location));
    }

    window.addEvent('unload', function(ev) {
        if (transport) {
            transport.close();
            transport = null;
        }
    });
}

window.addEvent("domready", function() {
    Carcassonne();
});
