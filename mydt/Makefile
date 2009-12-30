# vim:fdm=marker:nu:nowrap:noexpandtab

common_src=dt.c mem_pool.c treap.c

libmydt.a: ${common_src}
	gcc -c -lm ${common_src} && ar rcs libmydt.a ${common_src:.c=.o} && mkdir -p build && mv libmydt.a build && cp *.h build

clean:
	rm -rf *.o

veryclean: clean
	rm -rf build
	
