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
        toID: function() {
            return this.createId;
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
        inject: function(cont) {
            this.shadow.inject(cont);
            this.el.inject(cont);
        },
        /*
        showShadowAt: function(coord) {
            this.shadow.setPosition(coord).setStyles('display', 'block').store('coord', coord);
        },
        followShadow: function() {
            var coord = this.shadow.retrieve('coord');
            if (!coord)
                return;
            this.el.morph({left: coord.x, top: coord.y});
        },*/
        freeze: function() {
            this.frozed = true;
            var currMapName = mapName(this.createId, this.rotation);
            $each(this.el.getElements("map"), function(m) { 
                if (m.getProperty('name') != currMapName)
                    m.dispose(); 
            });
            this.shadow.dispose();
        },
        setFit: function() {
            if (this.frozed)
                return;
            this.img.setProperty('src', tileTransparentUrl);
            this.shadow.setProperty('src', tileTransparentUrl);
            this.fit = true;
        },
        setUnFit: function() {
            if (this.frozed)
                return;
            this.img.setProperty('src', tileTransparentRedUrl);
            this.shadow.setProperty('src', tileTransparentRedUrl);
            this.fit = false;
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
        findTile: function(tile) {
            return this.tiles.find(tile);
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
            var c = this.findTile(tile);
            if (!c)
                return;
            var gX = c.gX;
            var gY = c.gY;
            this.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors -1 count
                return (v || 0) - 1;
            });
            this.tiles.erase(gX, gY);
            --this.tilesCnt;
        },
        addTile: function(c, tile) {
            if (!this.canPlace(c.gX, c.gY))
                return false;

            this.rmTile(tile);

            var gX = c.gX;
            var gY = c.gY;
            this.neighborCnt.applyNeighbor(gX, gY, function(v, d) {                 // all neighbors +1 count
                return (v || 0) + 1;
            });
            this.tiles.set(gX, gY, tile);
            this.checkFit(tile);
            ++this.tilesCnt;
            return true;
        },
        checkFit: function(tile) {
            var c = this.findTile(tile);
            if (!c)
                return;
            var fit = true;
            this.tiles.applyNeighbor(c.gX, c.gY, function(t, d) {
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
