CC := gcc
FLAGS := -Iinclude/fract -ggdb -static-libgcc
LIBS := -lslavio

.PHONY:test
test:
	$(CC) $(FLAGS) -o test/test test/test.c src/*.c $(LIBS)