CC := gcc
FLAGS := -Iinclude/fract -ggdb -static-libgcc
LIBS := -lslavio -ldatam -lm

.PHONY:test
test:
	$(CC) $(FLAGS) -o test/test test/test.c $(filter-out src/cli.c,$(wildcard src/*.c)) $(LIBS)

.PHONY:cli
cli:
	$(CC) $(FLAGS) -o cli src/*.c $(LIBS)

.PHONY:mineg
mineg:
	$(CC) $(FLAGS) -o test/mineg test/mineg.c $(filter-out src/cli.c,$(wildcard src/*.c)) $(LIBS)