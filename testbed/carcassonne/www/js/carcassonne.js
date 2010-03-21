// vim::nu

/* carcassonne game logic */
/* required: 
 *     mootool-core.js
 *     res.js
 */

/* Organize all uniq identified object as a tree
 * objs will be deleted when its parent is deleted
 */
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
        return this;
    },
    show: function() {
        this.setStyle('display', 'block');
        return this;
    },
    hide: function() {
        this.setStyle('display', 'none');
        return this;
    }
});

Element.Events.spacePressed = {
    base: 'keyup',
    condition: function(ev) {
        return ev.key == 'space' && !ev.alt && !ev.control && !ev.shift;
    }
};

Element.Events.cntrlClick = {
    base: 'click',
    condition: function(ev) {
        return ev.control;
    }
};

if (!window.WebSocket) {
    window.addEvent('domready', function() {
        $("browserNotSupport").show();
    });
} else {

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
             * onPut: function(meeple)
             * onPick: function(meeple)
             * */
        },

        initialize: function(id, colorID, tile, meepleType, showPos, opt) {
            this.setOptions(opt);

            // attr
            if (tile.meeple)
                tile.meeple.finalize();
            this.tile = tile;
            tile.meeple = this;
            this.colorID = colorID;

            this.tile.freeze();

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
            this.el.inject(tile);
            this.show(meepleType, showPos);
            this.parent(id);
            this.fireEvent('put', [this]);
        },
        finalize: function() {
            this.tile.meeple = null;
            this.tile.unfreeze();
            this.tile = null;
            this.el.destroy();
            this.parent();
            this.fireEvent('pick', [this]);
        },
        toElement: function() {
            return this.el;
        },
        show: function(meepleType, pos) {
            pos = {x: pos.x - halfMeepleImgSize, y: pos.y - halfMeepleImgSize};
            this.el.setPosition(pos);
            this.el.setStyle('display', 'block');
            this.pos = pos;
            this.el.setProperty('title', meepleType);
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
             * onMeepleCreated: function(meeple)
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
            this.permanent = false;
            this.notDragged = true;                                 // XXX a tag to tell between click and drag
            this.meeple = null;

            // shadow (shadow is used for positioning)
            this.shadow = new Element('div', {
                "border": "0",
                "styles": {
                    "padding": "0px",
                    "background-image": "url('" + tilesImgUrl + "')",
                    "background-repeat": "no-repeat",
                    "width": tileImgSize + "px",
                    "height": tileImgSize + "px",
                    "position": "absolute",
                    "opacity": 0.5
                }
            });

            // DOM: create container div and the img ( for image map )
            this.el = new Element('div', {
                "styles": {
                    "position": "absolute"                          // import otherwise z-index not work
                }
            });
            this.el.store('tile', this);
            this.el.set('morph', {duration: 'short', transition: Fx.Transitions.Sine.easeOut});
            this.img = new Element('img', {
                "border": "0",
                "src": tileTransparentUrl,                          // FF needs a src, otherwise image map will not work
                "styles": {
                    "padding": "0px",
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

            this.el.setStyle("zIndex", 900);
            this.shadow.setStyle("zIndex", 800);

            // init rotation
            this.rotate();
        },
        finalize: function() {
            if (this.meeple)
                this.meeple.finalize();
            if (this.el)
                this.el.destroy();
            if (this.shadow)
                this.shadow.destroy();
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
            r = r || 0;
            // update rotation
            this.rotation = r % 4;

            // update background image
            var bgPos = "{x}px {y}px".substitute({ x: -this.tileIdx * tileImgSize, y: -this.rotation * tileImgSize })
            this.img.setStyle('background-position', bgPos);
            this.shadow.setStyle('background-position', bgPos);

            // update map
            this.img.setProperty('usemap', '#' + mapName(this.toID(), this.rotation));
            this.fireEvent('rotate', [this, this.rotation]);
        },
        createMeeple: function(meepleID, colorID, meepleType, showPos, opt) {
            var meeple = new Meeple(meepleID, colorID, this, 
                meepleType, showPos, opt);
            this.fireEvent('meeplecreated', [meeple]);
        },
        freeze: function(permanent) {        
            if (this.permanent)
                return;
            this.stopDraggable();
            this.frozed = true;
            if (permanent)
                this.permanent = true;
        },
        unfreeze: function() {        
            if (this.permanent)
                return;
            this.frozed = false;
            this.makeDraggable();
        },
        stop: function() {
            this.freeze(true);

            // remove unused parts
            this.el.getElements('map').each(function(m) {
                m.destroy();
            });
            this.img.removeProperty('usemap');

            if (this.shadow) {
                this.shadow.destroy();
                this.shadow = null;
            }

            // remove events
            if (this.dragger) {
                this.dragger.stop();
                this.dragger = null;
            }
            this.removeEvents();

            if (this.meeple) {
                $(this.meeple).removeEvents('click');
            }
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

        Implements: [Events, Options],

        options: {
            /*
             * onTileCreated: function(tile)
             * */
        },

        initialize: function() {
            // attr
            this.neighborCnt = new Grid();                      // (x, y) -> neighbor count
            this.tiles = new Grid();                            // (x, y) -> tile
            this.tilesCnt = 0;
            this.currTile = null;

            // DOM
            this.el = new Element('div', {
                'styles': {
                    'outline-style': 'dotted',
                    'border': 'none',                           // no border for caculating coord simpler
                    'position': 'absolute',
                    'width': tileImgSize + "px",
                    'height': tileImgSize + "px"
                }
            });
        },
        finalize: function() {
            this.currTile = null;
            this.tiles.each(function(t) {
                t.finalize();
            });
            if (this.el)
                this.el.destroy();
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
        ensureGridCoord: function(c) {
            if (!$chk(c.x)) {
                if (!$chk(c.gX))
                    throw "bad arg";
                c.x = c.gX * tileImgSize;
                c.y = - c.gY * tileImgSize;
            }
            else if (!$chk(c.gX)) {
                if (!$chk(c.x))
                    throw "bad arg";
                c.gX = (c.x/tileImgSize).toInt();
                c.gY = - (c.y/tileImgSize).toInt();
            }
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
            this.ensureGridCoord(c);

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
            this.currTile = tile;
            this.fireEvent('tilecreated', [tile]);
            return tile;
        },
        stopCurrTile: function() {
            if (!this.currTile)
                return;
            this.currTile.stop();
        },
        takeTurn: function() {
            this.currTile = null;
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
            this.currPlayer = null;

            var inst = this;
            $('mvCenter').addEvent('click', function(ev) {
                inst.fireEvent('mvcentclick', []);
            });
            $('turnEnd').addEvent('click', function(ev) {
                inst.fireEvent('turnendclick', []);
            });

            function onResize() {
                var winH = window.getSize().y;
                $("msgHistory").setStyle('height', (winH - 180) + 'px');
            };
            onResize();
            window.addEvent('resize', onResize);

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
            this.resetGame();
        },
        addMeepleCnt: function(colorID, delta) {
            var mc = meepleCnt(colorID);
            mc.set('text', mc.get('text').toInt() + delta);
        },
        addScore: function(colorID, delta) {
            var sc = score(colorID);
            sc.set('text', sc.get('text').toInt() + delta);
        },
        decRemainTiles: function(delta) {
            delta = delta || 1;
            var rt = $('remainTiles');
            rt.set('text', rt.get('text').toInt() - delta);
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
            scoreBoard(colorID).removeClass('scoreBoardSelected');
            playerName(colorID).set('html', '');
        },
        selectColor: function(player, colorID) {                        // can only called by server
            if (this.players[colorID])
                throw "select a selected color";

            player.colorID = colorID;
            this.players[colorID] = player;

            // alter page content
            scoreBoard(colorID).addClass('scoreBoardSelected');
            if (player == Player.self)
                playerName(colorID).set('html', '<font color=red>' + player.nickname + '</font>');
            else
                playerName(colorID).set('text', player.nickname);
        },
        resetScoreBoard: function(colorID) {
            scoreBoard(colorID).removeClass('scoreBoardOnTurn');
            // meepleCnt(colorID).set('text', 7);
            score(colorID).set('text', 0);
            readySt(colorID).set('html', 'No');
        },
        resetGame: function() {
            this.endGame();
            for (var i = 0; i < 5; ++i)
                this.resetScoreBoard(i);
            this.currPlayer = null;
        },
        startGame: function(tileCnt) {
            $("remainTiles").set("text", tileCnt);
            $("gameInfo").show();
            $("mvCenter").show();
        },
        endGame: function() {
            $("remainTiles").set("text", 0);
            $("gameInfo").hide();
            $("mvCenter").hide();
            $("turnEnd").hide();
        },
        takeTurn: function(player) {
            if (this.currPlayer) {
                scoreBoard(this.currPlayer.colorID).removeClass('scoreBoardOnTurn');
                if (this.currPlayer == Player.self)
                    $("turnEnd").hide();
            }
            scoreBoard(player.colorID).addClass('scoreBoardOnTurn');
            this.currPlayer = player;
            if (this.isSelfTurn())
                $("turnEnd").show();
        },
        isSelfTurn: function() {
            return this.currPlayer == Player.self;
        }
    });
})();

var MsgPanel = (function() {
    
    function showMsgEl(el) {
        var history = $("msgHistory");
        el.inject(history);
        var historyH = history.getSize().y;
        var historyScrollH = history.getScrollSize().y;
        history.scrollTo(0, historyScrollH - historyH + 30);
    }

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
                var msg = $("msgContent").getProperty("value");
                if (!msg) {
                    alert("请不要发送空信息");
                    return false;
                }
                inst.fireEvent('chat', [msg]);
                inst.chatMsg(msg);
                $("msgContent").setProperty("value", "");
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
            $("msgContent").setProperty("value", "");
        },
        chatMsg: function(msg, player) {
            showMsgEl(new Element("p", {"text": (player ? player.nickname : "您") + ": " + msg
                }));
            $("msgContent").set("text", "");
        },
        sysMsg: function(msg) {
            showMsgEl(new Element("p", {"text": "系统: " + msg, 
                "styles": {
                    color: "red"
                }
            }));
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

function ephemeralText(cont, pos, text, color) {
    var el = new Element('div', {
        'styles': {
            'position': 'absolute',
            'font-weight': 'bolder',
            'font-family': 'sans-serif',
            'font-size': '30px',
            'zIndex': 1100,
            'color': color
        },
        'text': text,
        'morph': {
            'duration': 2000,
            'transition': 'pow:in:out',
            'onComplete': function() {
                el.destroy();
            }
        }
    });
    el.inject(cont);
    el.setPosition(pos);

    el.morph({
        'opacity': [1, 0]
    });
}

function Carcassonne() {
    var loginPanel, gamePanel, msgPanel, board, transport;

    loginPanel = new LoginPanel();
    gamePanel = new GamePanel();
    msgPanel = new MsgPanel();

    /************************
     * Helpers
     ************************/

    function reset(err) {
        if (board) {
            board.finalize();
            board = null;
        }
        hideResut();
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

    var takeCntrl, handOverCntrl;

    (function() {
        function __onTileCreated(tile) {
            tile.addEvent('put', function(t, coord) {
                _moveTile(coord);
            });
            tile.addEvent('rotate', function(t, rotation) {
                _rotateTile(rotation);
            });
            tile.addEvent('terraclick', function(t, terra, terraType, ev) {
                if (!t.fit)
                    return;
                
                var pos = $(t).getPosition();
                pos.x = ev.page.x - pos.x;
                pos.y = ev.page.y - pos.y;
                _putMeeple(terra, pos);
            });
            tile.addEvent('meeplecreated', function(m) {
                $(m).addEvent('click', function(ev) {
                    _pickMeeple();
                });
            });
            tile.makeDraggable();
        }
        function __onCntrlClick(ev) {
            var coord = board.getGridCoord(ev.page);
            if (board.occupied(coord))
                return;

            if (board.currTile) {
                board.currTile.moveTo(coord);
                return;
            }
            
            _putTile(coord);
        }
        function __onSpacePressed() {
            if (board.currTile)
                board.currTile.rotate(board.currTile.rotation + 1);
        }

        takeCntrl = function() {
            board.addEvent('tilecreated', __onTileCreated);
            window.addEvent('cntrlClick', __onCntrlClick);
            window.addEvent('spacePressed', __onSpacePressed);
        };

        handOverCntrl = function() {
            board.stopCurrTile();
            board.removeEvent('tilecreated', __onTileCreated);
            window.removeEvent('cntrlClick', __onCntrlClick);
            window.removeEvent('spacePressed', __onSpacePressed);
        };
    })();

    document.getElements('.closeResultBox').each(function(c) {
        c.addEvent('click', function(ev) {
            c.getParent().hide();
            return false;
        });
    });

    function youWin() {
        $("youWin").show().mvToPageCenter();
    }
    function youLose() {
        $("youLose").show().mvToPageCenter();
    };
    function hideResut() {
        $("youWin").hide();
        $("youLose").hide();
    };

    gamePanel.addEvent('mvcentclick', function() {
        if (board)
            $(board).mvToPageCenter();
    });

    /************************
     * RPCs called to server 
     ************************/

    function assertOK(fnName) {
        return function(res) {
            if (!res.ok)
                throw "result not ok: " + fnName;
        }
    }

    // called when enter a nickname in login panel and submit
    function _join(nickname, gameID) {
        transport.call('join', [nickname, gameID], function(res) {
            // res {ok: true/false, msg: x, selfID: x, players: [...]}
            if (!res.ok) {                                          
                loginPanel.showInput(res.msg);
                return;
            }

            loginPanel.hide();

            // res.players [{id: x, nickname: x, colorID: x, ready: true/false}, ...]
            $each(res.players, function(u) {                                              
                var p = new Player(u.id, u.nickname);
                if (u.id == res.selfID)
                    p.asSelf();
                gamePanel.selectColor(p, u.colorID);
                if (u.ready)
                    gamePanel.becomeReady(p);       
            });
            gamePanel.show();
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

    function _ready() {
        transport.call('ready', [], assertOK('ready'));
    }
    gamePanel.addEvent('ready', function() {
        // ready for the next game so destroy the last
        if (board) {
            board.finalize();
            board = null;
        }
        hideResut();

        _ready();
        msgPanel.sysMsg("您准备好了");
    });

    function _chat(msg) {
        transport.call('chat', [msg], assertOK('chat'));
    }
    msgPanel.addEvent('chat', _chat);

    function _putTile(coord) {
        transport.call('putTile', [coord.gX, coord.gY], assertOK('putTile'));
    }

    function _moveTile(coord) {
        transport.call('moveTile', [coord.gX, coord.gY], assertOK('moveTile'));
    }

    function _rotateTile(rotation) {
        transport.call('rotateTile', [rotation], assertOK('rotateTile'));
    }

    function _putMeeple(terra, pos) {
        transport.call('putMeeple', [terra, pos]);
    }

    function _pickMeeple() {
        transport.call('pickMeeple', [], assertOK('pickMeeple'));
    }

    function _turnEnd() {
        transport.call('turnEnd', [], assertOK('turnEnd'));
    }
    gamePanel.addEvent('turnendclick', function() {
        if (!board.currTile) {
            alert("这一回合还没放下方块");
            return;
        }
        
        if (!board.currTile.fit) {
            alert("方块的位置/方向不适合");
            return;
        }

        // stop further actions here immediately 
        handOverCntrl();
        _turnEnd();
    });


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
            p.finalize();
        },
        ready: function(id) {
            var player = UniqObj.fromID(id);
            gamePanel.becomeReady(player);
            msgPanel.sysMsg(player.nickname + " 准备好了");
        },
        chat: function(id, msg) {
            // if (id == Player.self.toID())                                                // filter out
            //    return false;
            msgPanel.chatMsg(msg, UniqObj.fromID(id));
        },
        sysMsg: function(msg) {
            msgPanel.sysMsg(msg);
        },
        startGame: function(tileCnt) {
            // create board
            board = new Board();
            board.addEvent('tilecreated', function() {
                gamePanel.decRemainTiles();
            });
            $(board).inject($("boardCont")).mvToPageCenter();
            gamePanel.startGame(tileCnt);
            msgPanel.sysMsg("游戏开始!! ");
        },
        takeTurn: function(playerID) {
            // some clean for the prev turn
            if (board.currTile) {
                gamePanel.isSelfTurn() ? handOverCntrl() : board.stopCurrTile();
                $(board).makeDraggable({'handle': $(board.currTile)});
            }

            // new turn
            var player = UniqObj.fromID(playerID);
            board.takeTurn();
            gamePanel.takeTurn(player);
            if (gamePanel.isSelfTurn())
                takeCntrl()

            msgPanel.sysMsg("新回合, 请 " + player.nickname + " 行动");
        },
        putTile: function(tileID, tileIdx, coord) {
            if (board.currTile)
                throw "there is a tile not handled";

            board.ensureGridCoord(coord);
            var tile = board.createTile(tileID, tileIdx, coord);
        },
        moveTile: function(coord) {
            if (!board.currTile)
                throw "no current tile";

            board.ensureGridCoord(coord);
            board.currTile.moveTo(coord);
        },
        rotateTile: function(rotation) {
            if (!board.currTile)
                throw "no current tile";

            board.currTile.rotate(rotation);
        },
        putMeeple: function(meepleID, colorID, tileID, meepleType, showPos) {
            var meeple = UniqObj.fromID(meepleID);
            if (meeple) {
                meeple.show(meepleType, showPos);
                return;
            }

            UniqObj.fromID(tileID).createMeeple(meepleID, colorID, meepleType, 
                    showPos, {
                onPut: function() {
                    gamePanel.addMeepleCnt(colorID, -1);
                },
                onPick: function() {
                    gamePanel.addMeepleCnt(colorID, 1);
                }
            });
        },
        pickMeeple: function(meepleID, score) {
            var meeple = UniqObj.fromID(meepleID);
            if (score) {
                ephemeralText(meeple.tile, meeple.pos, '+' + score, 
                    colorName[meeple.colorID]);
                gamePanel.addScore(meeple.colorID, score);
            }
            meeple.finalize();
        },
        gameEndResult: function(winers) {
            winers.contains(Player.self.toID()) ? youWin() : youLose();
            var winerNames = winers.map(function(wid) {
                return UniqObj.fromID(wid).nickname;
            });
            var m = "游戏结束";
            if (winerNames.length)
                m += ", 胜利者是: " + winerNames.join(', ');
            msgPanel.sysMsg(m);
        },
        cleanGame: function() {
            if (gamePanel.isSelfTurn())
                handOverCntrl();
            gamePanel.resetGame();
            gamePanel.makeReadyClickable();
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

}
