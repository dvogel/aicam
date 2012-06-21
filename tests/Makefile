
CC=gcc
CFLAGS=-Wall -std=c99 -I. -I.. -lpulse-simple
TESTS=test_movingavg

runtests: tests
	./test_movingavg

tests: test_movingavg

test_movingavg: test_movingavg.c ../movingavg.c
	$(CC) $(CFLAGS) -o test_movingavg test_movingavg.c ../movingavg.c





