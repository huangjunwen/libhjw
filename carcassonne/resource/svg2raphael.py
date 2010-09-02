import sys, re
import warnings
import xml.etree.ElementTree as ET

class Svg2Raphael(object):
    """
    Extend this class directly if more svg xml tag needs to be support
    """

    def __init__(self, funcName):
        self.funcName = funcName
        self.contVarName = "canvas"

    transformPat = re.compile(r"(\w+)\([^\)]+\)")
    raphaelTransform = ("rotate", "translate", "scale")
    def _getElemAttr(self, elem):
        attr = elem.attrib
        style = attr.get("style", "")
        if style:
            style = '.attr("' + style.replace(':', '","').replace(';','").attr("') + '")'

        # http://www.w3.org/TR/SVG/coords.html#TransformAttribute
        transform = attr.get("transform", "")
        if transform:
            t = []
            for m in self.transformPat.finditer(transform):
                if m.group(1) not in self.raphaelTransform:
                    warnings.warn("raphael not support transform %s" % m.group(1))
                    continue
                t.append(m.group(0))
            transform = "." + ".".join(t)
        return style + transform

    def svg(self, elem, childRes):
        sep = ";"
        tmpl = sep.join(["function %s(contID){var %s=Raphael(contID,%s,%s)", 
            "%s", 
            "return %s", 
            "}"]
        )
        attr = elem.attrib
        return tmpl % (self.funcName, self.contVarName, attr['width'], 
            attr['height'], 
            sep.join(childRes),
            self.contVarName,
        )
    
    def g(self, elem, childRes):
        if not childRes:
            return
        sep = ","
        return "%s.set().push(%s)%s" % (self.contVarName, 
            sep.join(childRes),
            self._getElemAttr(elem),
        )

    def rect(self, elem, _):
        attr = elem.attrib
        return "%s.rect(%s,%s,%s,%s)%s" % (self.contVarName,
            attr['x'],
            attr['y'],
            attr['width'],
            attr['height'],
            self._getElemAttr(elem),
        )
    
    def path(self, elem, _):
        attr = elem.attrib
        return "%s.path('%s')%s" % (self.contVarName,
            attr['d'],
            self._getElemAttr(elem),
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

def main():
    if len(sys.argv) != 2:
        print "Usage:"
        print "\tpython %s <svg file name>" % __file__
        return
    filename = sys.argv[1]
    svg = ET.parse(open(filename)).getroot()
    assert svg.tag == svgPrefix + "svg"
    return traverse(svg, Svg2Raphael("svg" + filename.split('.')[0]))


if __name__ == '__main__':
    print main()
