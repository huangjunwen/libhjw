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
        initialize: function(tileIdx) {
            // alias
            var inst = this;

            // attr
            this.createId = tileCreated++;
            this.tileIdx = tileIdx;
            this.rotation = -1;
            this.fit = null;
            this.board = null;                                      // set by board
            this.coordOnBoard = null;                               // set by board

            // DOM: create container div and the img ( for image map )
            this.el = new Element('div');
            this.el.store('tile', this);
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
                'display': 'none',
                'position': 'absolute',
                'opacity': 0.5
            });

            // init rotation
            this.rotate();
        },
        toElement: function() {                         // ref: http://n2.nabble.com/Extends-Element-in-1-2-td796845.html
            return this.el;
        },
        getAreas: function() {                          // find areas in all rotation
            return this.el.getElements('map area');
        },
        getBound: function(d) {
            return tilesBounds[this.tileIdx][(4 + d - this.rotation)%4];
        },
        getShadow: function() {
            return this.shadow;
        },
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
        rotate: function() {
            // update rotation
            this.rotation = (this.rotation + 1) % 4;

            // update background image
            var bgPos = "{x}px {y}px".substitute({ x: -this.tileIdx * tileImgSize, y: -this.rotation * tileImgSize })
            this.img.setStyle('background-position', bgPos);
            this.shadow.setStyle('background-position', bgPos);

            // update map
            this.img.setProperty('usemap', '#' + mapName(this.createId, this.rotation));
        }
    });
})();


var Board = (function() {

    var Grid = new Class({
        initialize: function() {
            this.d = new Hash({});
        },
        get: function(x, y) {
            var r = this.d.get(x);
            if (!r)
                return null;
            return r.get(y);
        },
        set: function(x, y, v) {
            if (!this.d.has(x))
                this.d[x] = new Hash({});
            this.d[x][y] = v;
        },
        apply: function(x, y, arg, f) {
            var r = f(this.get(x, y), arg);
            if (!$defined(r))
                return;
            this.set(x, y, r)
        },
        applyNeighbor: function(x, y, f) {                      // for each of the 4 neighbors
            this.apply(x, y + 1, 0, f);                         // up
            this.apply(x + 1, y, 1, f);                         // right
            this.apply(x, y - 1, 2, f);                         // down
            this.apply(x - 1, y, 3, f);                         // left
        }
    });

    return new Class({
        initialize: function() {
            // attr
            this.maxGridY = this.maxGridX = 1;
            this.minGridY = this.minGridX = -1;

            this.neighborCnt = new Grid();                      // x -> y -> neighbor count
            this.tiles = new Grid();
            this.tilesCnt = 0;

            // DOM
            this.el = new Element('div', {
                'styles': {
                    'background-color': '#e0e0e0',
                    'border': 'none',                           // no border for caculating coord simpler
                    'position': 'absolute',
                    'left': '300px',                            // XXX
                    'top': '300px',                             // XXX
                    'width': tileImgSize + "px",
                    'height': tileImgSize + "px"
                }
            });
        },
        toElement: function() {
            return this.el;
        },
        canPlace: function(gX, gY) {
            if (!this.tilesCnt && !gX && !gY)
                return true;
            if (this.tiles.get(gX, gY))
                return false;
            if ((this.neighborCnt.get(gX, gY) || 0) <= 0)
                return false;
            return true;
        },
        abs2GridCoord: function(c, relative) {                                  // c is the coord relative to the argument 'relative'
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
        abs2OpenGridCoord: function(c, relative) {
            c = this.abs2GridCoord(c, relative);
            if (!this.canPlace(c.gX, c.gY))
                return null;
            return c;
        },
        rmTile: function(tile) {
            if (tile.board != this)
                return;
            var gX = tile.coordOnBoard.gX;
            var gY = tile.coordOnBoard.gY;
            this.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors -1 open ref count
                return (v || 0) - 1;
            });
            this.tiles.set(gX, gY, null);
            tile.board = null;
            tile.coordOnBoard = null;
            --this.tilesCnt;
        },
        addTile: function(c, tile) {
            if (!this.canPlace(c.gX, c.gY))
                return false;

            this.rmTile(tile);

            var gX = c.gX;
            var gY = c.gY;
            this.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors +1 open ref count
                return (v || 0) + 1;
            });
            this.tiles.set(gX, gY, tile);
            tile.board = this;
            tile.coordOnBoard = c;
            ++this.tilesCnt;
            return true;
        },
        checkFit: function(tile) {
            if (tile.board != this)
                throw "Tile not on board";
            var fit = true;
            this.tiles.applyNeighbor(tile.coordOnBoard.gX, tile.coordOnBoard.gY, function(t, d) {
                if (!t)
                    return;
                if (tile.getBound(d) != t.getBound((d + 2)%4))
                    fit = false;
            });

            fit ? tile.setFit() : tile.setUnFit();
            return fit;
        }
    }); 
})();
