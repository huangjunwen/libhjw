#!/usr/bin/env python

infile = open("in.1.ele")
outfile = open("edges", 'w')

s = set()

for line in infile:
    line = line.split()
    if len(line) != 4:
        continue
    line = map(int, line[1:])
    line.sort()
    a, b, c = line
    s.add((a, b))
    s.add((b, c))
    s.add((a, c))

l = list(s)
l.sort(lambda x, y: cmp(x[0], y[0]))

for a, b in l:
    outfile.write("%d %d\n" % (a, b))

