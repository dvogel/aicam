
CC=gcc
CFLAGS=-Wall -std=c99 -D_POSIX_C_SOURCE -I. -I/usr/include/alsa -lasound
OBJS=movingavg.o

all: recording master

tests: tests/all
	make -C tests

recording: $(OBJS) alsarec.o
	$(CC) $(CFLAGS) -orecording $(OBJS) alsarec.o

master: $(OBJS) master.o
	$(CC) $(CFLAGS) -omaster $(OBJS) master.o



