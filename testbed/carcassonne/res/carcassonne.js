// vim::nu

/* carcassonne game logic */
/* required: 
 *     mootool-core.js
 *     res.js
 * provide:
 *     Class Tile
 */

/* Organize all items( uniq identified item ) as a tree
 * items will be deleted when its parent is deleted
 */
var Item = (function() {
    var allItems = new Hash();

    var ret = new Class({
        initialize: function(id) {
            var item = allItems.get(id);
            if (item)
                throw "item " + id + " already created";

            this.itemID = id;
            this.parentItem = null;
            this.childItems = new Hash();
            allItems.set(id, this);
        },
        toID: function() {
            return this.itemID;
        },
        addChild: function(item) {
            if (!item.toID)
                item = ret.fromID(item);
            
            if (!item)
                throw "bad child";

            if (item.parentItem)
                throw "item " + item.toID() + " already has parent";

            this.childItems.set(item.toID(), item);
            item.parentItem = this;
        },
        del: function() {               // del will delete all child items as well
            ret.del(this.toID());
        }
    });
    $extend(ret, {
        fromID: function(id) {
            return allItems.get(id);
        },
        del: function(id) {
            var item = allItems.get(id);
            if (!item)
                return;

            // remove child/parent link
            if (item.parentItem)
                item.parentItem.childItems.erase(item.toID());
            item.parentItem = null;

            $each(item.childItems.getValues(), function(child) {      // XXX is this copy out necessary? (child.del will modify childItems itself)
                child.del();
            });

            // remove from global
            allItems.erase(id);
            delete item;
        },
        clearAll: function() {
            allItems = new Hash();
        }
    });
    return ret;
})();


var Tile = (function() {

    // private methods

    var coordConvs = [function(x,y){return [x,y];}, function(x,y){return [tileImgSize-y,x];}, 
        function(x,y){return [tileImgSize-x,tileImgSize-y];}, 
        function(x,y){return [y,tileImgSize-x];}
    ];

    var mapName = function(itemID, rotation) {
        return 'TM' + itemID + '_' + rotation;
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

        Extends: Item,

        Implements: [Events, Options],

        options: {
            /*
             * onPicked: function(tile)
             * onDrag: function(tile, ev)
             * onPut: function(tile, pos)
             * onRotate: function(tile)
             *
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
                    (new Element('area', {shape: a.shape, terra: a.terra, alt: a.title,
                        title: a.title,
                        nohref: true,
                        coords: coords
                    })).inject(map);
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
            function end(el) {
                inst.fireEvent('put', [inst, inst.targetPos]);
                morphToTargetPos.call(inst);
            }
            this.dragger = new Drag.Move($(inst), {
                onStart: function(el) {
                    inst.fireEvent('picked', [inst]);
                },
                onDrag: function(el, ev) {
                    inst.fireEvent('drag', [inst, ev]);
                },
                onComplete: end,
                onCancel: end
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
            /* 
             * freeze actions and release some contents 
             * */
            this.frozed = true;
            var currMapName = mapName(this.toID(), this.rotation);
            $each(this.el.getElements("map[name!=" + currMapName + "]"), function(m) { 
                m.dispose(); 
            });
            this.shadow.dispose();
            this.shadow = null;
            this.stopDraggable();
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

        Extends: Item,

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
            var fit = true;
            this.tiles.applyNeighbor(c.gX, c.gY, function(t, d) {
                if (!t)
                    return;
                if (tile.getBound(d) != t.getBound(boundOpposite[d]))
                    fit = false;
            });

            return fit;
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
                    if (board.tilesCnt == 1 && !c.gX && !c.gY) {
                        t.setFit();
                        return;
                    }
                    else if (board.getNeighborCnt(c) == 0 ||
                            !board.checkBounds(c, t)) {
                        t.setUnFit();
                        return;
                    }
                    t.setFit();
                    return;
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

/*
var Meeple = new Class({
});

var Carcassonne = new Class({
    initialize: function() {
        this.items = Hash();                                            // item id -> item
    }
});

var Player = new Class({
});

var Room = new Class({
    initialize: function() {
    }
});
*/
