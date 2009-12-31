curr_dir=`pwd`
lib=${curr_dir}/../build/libmydt.a

test: test.c
	gcc -o test -lm test.c ${lib}

clean:
	rm -rf test

veryclean: clean
	rm -rf in.node
