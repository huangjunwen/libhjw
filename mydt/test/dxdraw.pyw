import array
import math

import d3d, d3dx
from d3dc import *
        
pointlist = []
linelist = []

def loaddata():
	global pointlist, linelist
	pointlist = []
	linelist = []
	try:
		incontent = open("in.txt").readlines()
		l = int(incontent[0])
		for c in incontent[1:]:
			x, y = map(float, c.strip().split(' '))
			pointlist.append((x, y))
		assert len(pointlist) == l
	except:
		pass

	try:
		outcontent = open("out.txt").readlines()
		for c in outcontent:
			p1, p2 = eval(c)
			color = 0xffff0000
			linelist.append((p1[0], p1[1], 0.0, 1.0, color))
			linelist.append((p2[0], p2[1], 0.0, 1.0, color))
	except:
		pass
loaddata()

font = None

#See the FAQ 
fakefullscreen = False

class dt(d3dx.Frame):
	def __init__(self, *args, **kwargs):
		global font
		super(dt, self).__init__(*args, **kwargs)
		font = d3d.Font(u"xxx", 13, True)
		self.ll = []
	def onKey(self, msg):
		if msg[0] == WM.KEYDOWN:
			if msg[1] == VK.BACK:
				if len(self.ll):
					self.ll = self.ll[:-2]
			elif msg[1] == VK.SPACE:
				l = len(self.ll)
				self.ll.extend(linelist[l: l+2])
			elif msg[1] ==  VK.F11:
				l = len(self.ll)
				self.ll.extend(linelist[l:])
			elif msg[1] == VK.F12:
				loaddata()
				self.ll = []
	def onRender(self):
		global font
		d3d.setState(RS.FVF, FVF.XYZRHW | FVF.DIFFUSE)
		d3d.drawVertices(TYPE.LINELIST, self.ll)   
		for x, y in pointlist:
			t = unicode("(%d, %d)" % (int(x), int(y)))
			d3d.drawTexts(font, ((t, int(x), int(y), self.client[0] - 20, self.client[1] - 20, 0xffffff00),))


if __name__ == "__main__":
	frame = dt(u"Draw delaunay triangulation") 
	frame.mainloop()
