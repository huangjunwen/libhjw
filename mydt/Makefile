
top_dir=`pwd`
external_dir=${top_dir}/codebase
build_dir=${top_dir}/build

external_src=${external_dir}/mem_pool.c
src=dt.c treap.c ${external_src}

libmydt.a:
	mkdir -p ${build_dir} && gcc -c ${src} && ar rcs ${build_dir}/libmydt.a *.o && make clean

clean:
	rm -rf *.o

veryclean: clean
	rm -rf ${build_dir}

