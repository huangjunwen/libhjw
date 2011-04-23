
SRC=test.c scgi.c
INC=scgi.h
OBJ=$(SRC:.c=.o)
CC=gcc
CFLAG=-Wall

test: $(OBJ) $(INC)
	$(CC) $(CFLAG) -o $@ $(OBJ)

$(OBJ):
	$(CC) $(CFLAG) -o $@ -c $(@:.o=.c)

.PHONY: clean

clean:
	rm -rf $(OBJ) test

