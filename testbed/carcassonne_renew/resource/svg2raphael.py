import sys, re
import warnings
import xml.etree.ElementTree as ET

class Svg2RaphaelHandler(object):
    """
    Extend this class directly if more svg xml tag needs to be support
    """

    def __init__(self, funcName=""):
        self.funcName = funcName
        self.transformPat = re.compile(r"(\w+)\([^\)]+\)")
        self.raphaelTransforms = ("rotate", "translate", "scale")

    def _getElemAttr(self, elem):
        attr = elem.attrib

        # http://raphaeljs.com/reference.html#attr
        style = attr.get("style", "")
        if style:
            style = '.attr("' + style.replace(':', '","').replace(';','").attr("') + '")'

        # http://www.w3.org/TR/SVG/coords.html#TransformAttribute
        # http://raphaeljs.com/reference.html#rotate ...
        transform = attr.get("transform", "")
        if transform:
            t = []
            for m in self.transformPat.finditer(transform):
                if m.group(1) not in self.raphaelTransforms:
                    warnings.warn("raphael not support transform %s" % m.group(1))
                    continue
                t.append(m.group(0))
            transform = "." + ".".join(t)
        return style + transform

    def svg(self, elem, childRes):
        # convert <svg> to <g> so that one canvas can hold multiple objects.
        return "function %s(canvas){return canvas.set().push(%s);}" % (
            self.funcName, 
            ','.join(childRes),
        )
    
    def g(self, elem, childRes):
        if not childRes:
            return
        return "canvas.set().push(%s)%s" % (','.join(childRes),
            self._getElemAttr(elem),
        )

    def rect(self, elem, _):
        attr = elem.attrib
        return "canvas.rect(%s,%s,%s,%s)%s" % (attr['x'],
            attr['y'],
            attr['width'],
            attr['height'],
            self._getElemAttr(elem),
        )
    
    def path(self, elem, _):
        attr = elem.attrib
        return "canvas.path('%s')%s" % (attr['d'], self._getElemAttr(elem),
        )
    
    # ... more ...


svgNS = "http://www.w3.org/2000/svg"
svgPrefix = "{%s}" % svgNS

def traverse(elem, handler):
    if not elem.tag.startswith(svgPrefix):
        return

    tag = elem.tag[len(svgPrefix):]
    h = getattr(handler, tag, None)
    if h is None:
        warnings.warn("ignore svg tag %s" % tag)
        return

    childRes = [traverse(child, handler) for child in elem.getchildren()]
    return h(elem, [r for r in childRes if r is not None])

def svg2raphael(f, *args, **kw):
    """
    @param f: a svg file object
    @param args, kw: see Svg2RaphaelHandler's __init__
    @return: javascript function code to produce the svg file
    """
    svg = ET.parse(f).getroot()
    assert svg.tag == svgPrefix + "svg"
    return traverse(svg, Svg2RaphaelHandler(*args, **kw))

def main():
    if len(sys.argv) != 2:
        print "Usage:"
        print "\tpython %s <svg file name>" % __file__
        return
    filename = sys.argv[1]
    return svg2raphael(open(filename), "svg" + filename.split('.')[0])

if __name__ == '__main__':
    print main()
