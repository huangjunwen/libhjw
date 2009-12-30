pyver=2.5
all_src=pyconsole.cpp pyconsole.h test.cpp

pyconsole: ${all_src}
	g++ -o pyconsole -I/usr/include/python${pyver} -lpython${pyver} ${all_src}
