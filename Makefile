CC := gcc
FLAGS := -Iinclude/fract -ggdb -static-libgcc
LIBS := -lslavio -ldatam

.PHONY:test
test:
	$(CC) $(FLAGS) -o test/test test/test.c src/*.c $(LIBS)

.PHONY:cli
cli:
	$(CC) $(FLAGS) -o cli src/*.c $(LIBS)