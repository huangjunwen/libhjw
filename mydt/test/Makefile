lib_dir=../
common_src=${lib_dir}dt.c ${lib_dir}mem_pool.c ${lib_dir}treap.c

test: ${common_src} test.c
	gcc -o test -lm ${common_src} test.c

clean:
	rm test in.node
