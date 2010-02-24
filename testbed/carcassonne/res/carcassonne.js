// vim::nu

/* carcassonne game logic */
/* required: 
 *     mootool-core.js
 *     res.js
 * provide:
 *     Class Tile
 */

var Tile = (function() {

    var tileCreated = 0;

    var coordConvs = [function(x,y){return [x,y];}, function(x,y){return [tileImgSize-y,x];}, 
        function(x,y){return [tileImgSize-x,tileImgSize-y];}, 
        function(x,y){return [y,tileImgSize-x];}
    ];

    var mapName = function(createId, rotation) {
        return 'TM' + createId + '_' + rotation;
    };

    return new Class({
        Implements: [Events, Options],

        options: {
            /*
             * onPicked: function(tile, gridCoord)
             * onPut: function(tile, gridCoord)
             * onRotate: function(tile)
             * onFreeze: function(tile)
             *
             * */
        },

        initialize: function(tileIdx, opt) {
            this.setOptions(opt);
            // alias
            var inst = this;

            // attr
            this.createId = tileCreated++;
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
                var map = new Element('map', {'name': mapName(inst.createId, r)});
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
        toID: function() {
            return this.createId;
        },
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
        freeze: function() {        
            /* 
             * freeze actions and release some contents 
             * */
            this.frozed = true;
            var currMapName = mapName(this.createId, this.rotation);
            $each(this.el.getElements("map[name!=" + currMapName + "]"), function(m) { 
                m.dispose(); 
            });
            this.shadow.dispose();
            this.shadow = null;
            this.fireEvent('freeze', this);
        },
        injectAt: function(cont, pos) {
            if (this.frozed)
                return;
            this.shadow.inject(cont);
            this.showShadowAt(pos);
            this.el.inject(cont).setPosition(pos);
        },
        showShadowAt: function(pos) {
            if (this.frozed)
                return;
            this.shadow.setPosition(pos).store('pos', pos);
        },
        followShadow: function() {
            if (this.frozed)
                return;
            var pos = this.shadow.retrieve('pos');
            this.el.morph({left: pos.x, top: pos.y});
            return this.el.get('morph');
        },
        moveTo: function(pos) {
            if (this.frozed)
                return;
            this.showShadowAt(pos);
            return this.followShadow(pos);
        },
        rotate: function() {
            if (this.frozed)
                return;
            // update rotation
            this.rotation = (this.rotation + 1) % 4;

            // update background image
            var bgPos = "{x}px {y}px".substitute({ x: -this.tileIdx * tileImgSize, y: -this.rotation * tileImgSize })
            this.img.setStyle('background-position', bgPos);
            this.shadow.setStyle('background-position', bgPos);

            // update map
            this.img.setProperty('usemap', '#' + mapName(this.createId, this.rotation));
            this.fireEvent('rotate', this);
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
        initialize: function() {
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
        abs2GridCoord: function(c, relative) {                                      // c is the coord relative to the argument 'relative'
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
        findTile: function(tile) {
            var c;
            if (!(c = this.tiles.find(tile)))
                return null;
            c.x = c.gX*tileImgSize;
            c.y = c.gY*tileImgSize;
            return c;
        },
        pickTile: function(tile) {
            var c = this.findTile(tile);
            if (!c)
                return false;
            var gX = c.gX;
            var gY = c.gY;
            this.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors -1 count
                return (v || 0) - 1;
            });
            this.tiles.erase(gX, gY);
            --this.tilesCnt;
            tile.fireEvent('picked', [tile, c]);
            return true;
        },
        putTile: function(c, tile) {
            if (this.occupied(c))
                return false;

            this.pickTile(tile);                                                    // first pick up if not yet

            var gX = c.gX;
            var gY = c.gY;
            this.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors +1 count
                return (v || 0) + 1;
            });
            this.tiles.set(gX, gY, tile);
            ++this.tilesCnt;
            tile.fireEvent('put', [tile, c]);
            return true;
        },
        createTile: function(tileIdx, c, opt) {                                     // c should be a grid coord
            if (this.occupied(c))
                return null;

            var tile = new Tile(tileIdx, opt);
            tile.injectAt(this, c);
            this.putTile(c, tile);
            return tile;
        },
        makeTileDraggable: function(tile) {
            var c = this.findTile(tile);
            var board = this;

            function onEnd(el) {
                board.putTile(c, tile);
                tile.followShadow();
            }

            return $(tile).makeDraggable({
                onStart: function(el){
                    board.pickTile(tile);
                },
                onDrag: function(el, ev) {
                    var newCoord = board.abs2GridCoord(ev.page);
                    if (board.occupied(newCoord))
                        return;
                    c = newCoord;
                    tile.showShadowAt(c);
                },
                onComplete: onEnd,
                onCancel: onEnd
            });
        },
        moveTile: function(c, tile) {
            if (!this.putTile(c, tile))
                return null;
            return tile.moveTo(c);
        },
        checkBounds: function(c, tile) {
            var fit = true;
            this.tiles.applyNeighbor(c.gX, c.gY, function(t, d) {
                if (!t)
                    return;
                if (tile.getBound(d) != t.getBound(boundOpposite[d]))
                    fit = false;
            });

            // fit ? tile.setFit() : tile.setUnFit();
            return fit;
        }
    }); 
})();
