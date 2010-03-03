var tileImgSize = 108;

var tilesImgUrl = 'carcassonne.jpg';

var tileTransparentUrl = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGwAAABsCAYAAACPZlfNAAAAAXNSR0IArs4c6QAAAERJREFUeNrtwTEBAAAAwqD1T20IX6AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB4DbasAAEBrI08AAAAAElFTkSuQmCC';

var tileTransparentRedUrl = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGwAAABsCAMAAAC4uKf/AAAABGdBTUEAAK/INwWK6QAAABl0RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAAAGUExURf9/f////wnD88UAAAACdFJOU/8A5bcwSgAAAElJREFUeNrszwENAAAMw6Dev+n7WAgKqBuVmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmZmdmMF2AAFyAWyT9Vw1cAAAAASUVORK5CYII=';

var tilesMap = [[{"shape":"rect","coords":[[0,0],[48,48]],"terra":0,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,0],[60,48]],"terra":1,"title":"\u9053\u8def"},{"shape":"rect","coords":[[60,0],[108,48]],"terra":2,"title":"\u519c\u7530"},{"shape":"rect","coords":[[60,48],[108,60]],"terra":3,"title":"\u9053\u8def"},{"shape":"rect","coords":[[60,60],[108,108]],"terra":4,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,60],[60,108]],"terra":5,"title":"\u9053\u8def"},{"shape":"rect","coords":[[0,60],[48,108]],"terra":6,"title":"\u519c\u7530"},{"shape":"rect","coords":[[0,48],[48,60]],"terra":7,"title":"\u9053\u8def"}],
 [{"shape":"rect","coords":[[0,0],[108,108]],"terra":0,"title":"\u57ce\u5821"}],
 [{"shape":"rect","coords":[[36,24],[84,72]],"terra":1,"title":"\u4fee\u9053\u9662"},{"shape":"rect","coords":[[0,0],[108,108]],"terra":0,"title":"\u519c\u7530"}],
 [{"shape":"rect","coords":[[36,24],[84,72]],"terra":2,"title":"\u4fee\u9053\u9662"},{"shape":"rect","coords":[[36,72],[60,108]],"terra":1,"title":"\u9053\u8def"},{"shape":"rect","coords":[[0,0],[108,108]],"terra":0,"title":"\u519c\u7530"}],
 [{"shape":"rect","coords":[[0,0],[108,36]],"terra":0,"title":"\u519c\u7530"},{"shape":"rect","coords":[[60,36],[108,60]],"terra":1,"title":"\u9053\u8def"},{"shape":"rect","coords":[[60,60],[108,108]],"terra":2,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,60],[60,108]],"terra":3,"title":"\u9053\u8def"},{"shape":"rect","coords":[[0,60],[48,108]],"terra":4,"title":"\u519c\u7530"},{"shape":"rect","coords":[[0,36],[48,60]],"terra":5,"title":"\u9053\u8def"}],
 [{"shape":"poly","coords":[[0,0],[0,108],[48,72],[72,72],[108,108],[108,0]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,108],[48,72],[72,72],[108,108]],"terra":1,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[108,108],[72,72],[48,72],[0,108]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,108],[48,72],[72,72],[108,108]],"terra":1,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[108,108],[72,72],[48,72],[0,108]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[72,72],[108,108],[72,108]],"terra":1,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,72],[72,108]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[0,108],[48,72],[48,108]],"terra":3,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[108,108],[72,72],[48,72],[0,108]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[72,72],[72,108],[108,108]],"terra":1,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,72],[72,108]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[48,72],[48,108],[0,108]],"terra":3,"title":"\u519c\u7530"}],
 [{"shape":"rect","coords":[[0,0],[48,108]],"terra":0,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,0],[72,108]],"terra":1,"title":"\u9053\u8def"},{"shape":"rect","coords":[[72,0],[108,108]],"terra":2,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[0,108],[36,84],[36,60],[60,36],[108,0]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,108],[36,84],[36,60],[60,36],[108,0],[108,108]],"terra":1,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[60,36],[36,60],[36,84],[0,108]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[108,0],[108,108],[0,108],[36,84],[36,60],[60,36]],"terra":1,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[60,36],[36,60],[36,84],[0,108]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[108,0],[108,48],[72,72],[48,108],[0,108],[36,84],[36,60],[60,36]],"terra":1,"title":"\u519c\u7530"},{"shape":"poly","coords":[[108,48],[72,72],[48,108],[60,108],[108,60]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[60,108],[108,60],[108,108]],"terra":3,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[60,36],[36,60],[36,84],[0,108]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[108,0],[108,48],[72,72],[48,108],[0,108],[36,84],[36,60],[60,36]],"terra":1,"title":"\u519c\u7530"},{"shape":"poly","coords":[[72,72],[108,48],[108,60],[60,108],[48,108]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[60,108],[108,108],[108,60]],"terra":3,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[108,108],[60,108],[60,48],[0,48]],"terra":0,"title":"\u519c\u7530"},{"shape":"poly","coords":[[0,48],[60,48],[60,108],[48,108],[48,60],[0,60]],"terra":1,"title":"\u9053\u8def"},{"shape":"rect","coords":[[0,60],[48,108]],"terra":2,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[1,0],[108,0],[84,24],[48,24]],"terra":0,"title":"\u519c\u7530"},{"shape":"poly","coords":[[0,0],[48,24],[84,24],[108,0],[108,108],[84,84],[36,84],[0,108]],"terra":1,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,108],[36,84],[84,84],[108,108]],"terra":2,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[84,24],[36,24]],"terra":0,"title":"\u519c\u7530"},{"shape":"poly","coords":[[0,0],[36,24],[84,24],[108,0],[108,108],[84,84],[24,84],[0,108]],"terra":1,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[24,84],[84,84],[108,108],[0,108]],"terra":2,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[72,36],[36,36]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[36,36],[72,36],[108,0],[108,48],[72,48],[48,72],[48,108],[0,107]],"terra":1,"title":"\u519c\u7530"},{"shape":"poly","coords":[[108,48],[72,48],[48,72],[48,108],[60,108],[72,72],[108,60]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[72,72],[108,60],[108,108],[60,108]],"terra":3,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[48,36]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[48,36],[108,0],[108,108],[60,108],[60,72],[36,48],[0,48]],"terra":1,"title":"\u519c\u7530"},{"shape":"poly","coords":[[0,48],[36,48],[60,72],[60,108],[48,108],[36,72],[0,60]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[0,60],[36,72],[48,108],[0,108]],"terra":3,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[72,36],[36,36]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[36,36],[72,36],[108,0],[108,108],[0,108]],"terra":1,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[36,24]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[108,0],[72,36],[72,60],[108,108]],"terra":1,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[36,24],[108,0],[72,36],[72,60],[108,108],[0,108]],"terra":2,"title":"\u519c\u7530"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[72,24],[72,72],[108,108],[0,108],[36,72],[36,36]],"terra":0,"title":"\u519c\u7530"},{"shape":"poly","coords":[[108,0],[72,24],[72,72],[108,108]],"terra":1,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[36,36],[36,72],[0,108]],"terra":2,"title":"\u57ce\u5821"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[84,36],[24,36]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[24,36],[84,36],[108,0],[108,48],[60,60],[48,60],[0,48]],"terra":1,"title":"\u519c\u7530"},{"shape":"poly","coords":[[108,48],[108,60],[60,84],[60,60]],"terra":2,"title":"\u9053\u8def"},{"shape":"poly","coords":[[60,84],[60,108],[108,108],[108,60]],"terra":3,"title":"\u519c\u7530"},{"shape":"rect","coords":[[48,72],[60,108]],"terra":4,"title":"\u9053\u8def"},{"shape":"poly","coords":[[0,60],[48,72],[48,108],[0,108]],"terra":5,"title":"\u519c\u7530"},{"shape":"poly","coords":[[0,48],[48,60],[48,72],[0,60]],"terra":6,"title":"\u9053\u8def"}],
 [{"shape":"poly","coords":[[0,0],[108,0],[72,36],[24,36]],"terra":0,"title":"\u57ce\u5821"},{"shape":"poly","coords":[[0,0],[24,36],[72,36],[108,0],[108,48],[0,48]],"terra":1,"title":"\u519c\u7530"},{"shape":"rect","coords":[[0,48],[108,60]],"terra":2,"title":"\u9053\u8def"},{"shape":"rect","coords":[[0,60],[108,108]],"terra":3,"title":"\u519c\u7530"}]];


var tileBoundMap = {
    'FIELD': 1,
    'ROAD': 2,
    'CITY': 3
};

var tilesBounds = [
    [2,2,2,2], [3,3,3,3], [1,1,1,1], [1,1,2,1], [1,2,2,2], [3,3,1,3], [3,3,1,3], [3,3,2,3],
    [3,3,2,3], [2,1,2,1], [3,1,1,3], [3,1,1,3], [3,2,2,3], [3,2,2,3], [1,1,2,2], [1,3,1,3],
    [1,3,1,3], [3,2,2,1], [3,1,2,2], [3,1,1,1], [3,3,1,1], [1,3,1,3], [3,2,2,2], [3,2,1,2]
];

var meepleImgSize = 30;

var meepleImgUrl = 'meeple.png';
