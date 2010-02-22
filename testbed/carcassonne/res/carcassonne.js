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
            this.coordOnBoard = null;                               // set by board

            // create container div and the img ( for image map )
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
        getBounds: function() {
            var b = tilesBounds[this.tileIdx];
            var r = -this.rotation;
            return b.slice(r).extend(b.slice(0, r)); 
        },
        getShadow: function() {
            return this.shadow;
        },
        setFitStyle: function() {
            this.img.setProperty('src', tileTransparentUrl);
            this.shadow.setProperty('src', tileTransparentUrl);
            this.fit = true;
        },
        setNotFitStyle: function() {
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

    var Dict2D = new Class({
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
        apply: function(x, y, f) {
            var r = f(x, y, this.get(x, y));
            if (!$defined(r))
                return;
            this.set(x, y, r)
        },
        applyNeighbor: function(x, y, f) {                      // for each of the 8 neighbors
            this.apply(x - 1, y + 1, f);
            this.apply(x, y + 1, f);
            this.apply(x + 1, y + 1, f);
            this.apply(x + 1, y, f);
            this.apply(x + 1, y - 1, f);
            this.apply(x, y - 1, f);
            this.apply(x - 1, y - 1, f);
            this.apply(x - 1, y, f);
        }
    });

    return new Class({
        initialize: function() {
            // attr
            this.maxGridY = this.maxGridX = 1;
            this.minGridY = this.minGridX = -1;

            this.openGrid = new Dict2D();                       // x -> y -> open ref count
            this.openGrid.set(0, 0, 1);                         //   =0 not opened; >0 opened; <0 closed
            //this.openGrid.set(0, 1, 1);
            this.tiles = new Dict2D();

            // DOM
            this.el = new Element('div', {
                'styles': {
                    'background-color': '#e0e0e0',
                    'border': 'none',                           // no border for caculating coord simpler
                    'position': 'absolute',
                    'left': '300px',                            // XXX
                    'top': '300px',                             // XXX
                    'width': (tileImgSize*(this.maxGridX - this.minGridX + 1)) + "px",
                    'height': (tileImgSize*(this.maxGridY - this.minGridY + 1)) + "px"
                }
            });
        },
        toElement: function() {
            return this.el;
        },
        
        isGridOpen: function(gX, gY) {
            return (this.openGrid.get(gX, gY) || 0) > 0;
        },
        abs2OpenGridCoord: function(c) {                        // c is the absolute coord (relative to the document, e.g Event.page)
            var coord = this.el.getCoordinates();
            var x = c.x - coord.left;
            var y = c.y - coord.top;
            if (x < 0 || y < 0 || x >= coord.width || y >= coord.height)
                return null;
            x -= x%tileImgSize;
            y -= y%tileImgSize;
            gX = this.minGridX + (x/tileImgSize).toInt();
            gY = this.maxGridY - (y/tileImgSize).toInt();
            if (!this.isGridOpen(gX, gY))
                return null;
            return {'x': x, 'y': y, 'gX': gX, 'gY': gY};
        },
        addTile: function(c, tile) {
            var gX, gY;
            if (!this.isGridOpen(c.gX, c.gY))
                return false;

            if (tile.coordOnBoard) {
                gX = tile.coordOnBoard.gX;
                gY = tile.coordOnBoard.gY;
                this.openGrid.set(gX, gY, 1);
                this.openGrid.applyNeighbor(gX, gY, function(x, y, v) {             // all opened neighbors -1 open ref count
                    v = v || 0;
                    if (v > 0)
                        return v - 1;
                });
                this.tiles.set(gX, gY, null);
                tile.coordOnBoard = null;
            }

            gX = c.gX;
            gY = c.gY;
            this.openGrid.set(gX, gY, 0);
            this.openGrid.applyNeighbor(gX, gY, function(x, y, v) {                 // all not closed neighbors +1 open ref count
                v = v || 0;
                if (v >= 0)
                    return v + 1;
            });
            this.tiles.set(gX, gY, tile);
            tile.coordOnBoard = c;

            // need to enlarge the board
            if (gX == this.minGridX) {
            } else if (gX == this.maxGridX) {
            }

            if (gY == this.minGridY) {
            } else if (gY == this.maxGridY) {
            }
        }
    }); 
})();
