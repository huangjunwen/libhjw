#!/usr/bin/env python

from random import *

num = 50000
minx = 0.0
maxx = 1000.0
miny = 0.0
maxy = 700.0

f = open('in.txt', 'w')
xdelta = maxx - minx
ydelta = maxy - miny
for i in xrange(0, num):
	x, y = minx + random() * xdelta, miny + random() * ydelta
	f.write("%d %f %f\n" % (i, x, y))
f.close()
	


