CC := gcc
FLAGS := -Iinclude/fract
LIBS := -lslavio

.PHONY:test
test:
	$(CC) $(FLAGS) -o test/test test/test.c src/*.c $(LIBS)