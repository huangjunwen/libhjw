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
        },
        setNotFitStyle: function() {
            this.img.setProperty('src', tileTransparentRedUrl);
            this.shadow.setProperty('src', tileTransparentRedUrl);
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

    var halfTileImgSize = tileImgSize/2;

    return new Class({
        initialize: function() {
            this.gridHeight = this.gridWidth = 3;
            this.maxGridY = this.maxGridX = 1;
            this.minGridY = this.minGridX = -1;
            this.el = new Element('div', {
                'styles': {
                    'background-color': '#e0e0e0',
                    'border': 'none',                           // no border for caculating coord simpler
                    'position': 'absolute',
                    'left': '300px',
                    'top': '300px',
                    'width': (tileImgSize*this.gridWidth) + "px",
                    'height': (tileImgSize*this.gridHeight) + "px"
                }
            });
            this.openGrid = {'0_0': true};
        },
        toElement: function() {
            return this.el;
        },
        grid2Coord: function(g) {                       // grid to coord (lefttop corner relative to $(board))
            if (!g)
                return null;
            var gX = g.gX, gY = g.gY;
            if (gX < this.minGridX || gX > this.maxGridX || gY < this.minGridY || gY > this.maxGridY)
                return null;
            g.x = (gX - this.minGridX)*tileImgSize;
            g.y = (this.maxGridY - gY)*tileImgSize;
            return g;
        },
        coord2Grid: function(c) {                       // coord (lefttop corner relative to $(board)) to grid
            if (!c)
                return null;
            var x = c.x, y = c.y, coord = this.el.getCoordinates();
            if (x < 0 || y < 0 || x > coord.width || y > coord.height)
                return null;
            c.gX = this.minGridX + (x/tileImgSize).toInt();
            c.gY = this.maxGridY - (y/tileImgSize).toInt();
            return c;
        },
        abs2GridCoord: function(c) {                    // c is the absolute coord (relative to the document, e.g ev.page)
            var coord = this.el.getCoordinates();
            var x = c.x - coord.left;
            var y = c.y - coord.top;
            if (x < 0 || y < 0 || x >= coord.width || y >= coord.height)
                return null;
            return {'x': x - x%tileImgSize, 'y': y - y%tileImgSize};
        }
    }); 
})();
