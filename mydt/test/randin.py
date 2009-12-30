#!/usr/bin/env python

import sys
from random import *

try:
    num = int(sys.argv[1])
    assert num > 0
except:
    num = 50000

minx = 0.0
maxx = 1000.0
miny = 0.0
maxy = 700.0
xdelta = maxx - minx
ydelta = maxy - miny

print "generating %d point pairs to in.node..." % num
f = open('in.node', 'w')
f.write("%d 2 0 0\n" % num)
for i in xrange(0, num):
	x, y = minx + random() * xdelta, miny + random() * ydelta
	f.write("%d %f %f\n" % (i, x, y))
f.close()
print "done"
