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
                "padding": "0px 0px 0px 0px",
                "src": transparentUrl,
                "styles": {
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
                var map = new Element('map', {'name': inst.mapName(r)});
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

            // init rotation
            this.rotate();
        },
        toElement: function() {                   // ref: http://n2.nabble.com/Extends-Element-in-1-2-td796845.html
            return this.el;
        },
        mapName: function(rotation) {
            return 'TM' + this.createId + '_' + rotation;
        },
        areas: function() {                     // find areas in all rotation
            return this.el.getElements('map area');
        },
        rotate: function() {
            // update rotation
            this.rotation = (this.rotation + 1) % 4;

            // update background image
            this.img.setStyle('background-position', "{x}px {y}px".substitute({
                x: -this.tileIdx * tileImgSize,
                y: -this.rotation * tileImgSize
            }));

            // update map
            this.img.setProperty('usemap', '#' + this.mapName(this.rotation));
        }
    });
})();
