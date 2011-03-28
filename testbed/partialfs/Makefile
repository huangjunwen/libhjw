FUSE_CFLAGS:=`pkg-config fuse --cflags`
FUSE_LIBS:=`pkg-config fuse --libs`

partialfs: main.c partialfs.c radix.c
	gcc $(FUSE_CFLAGS) $(FUSE_LIBS) -Wall -o $@ $^


