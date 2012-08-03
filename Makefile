
CC=gcc
CFLAGS=-Wall -std=c99 -D_POSIX_C_SOURCE -I. -I/usr/include/alsa -lasound $(shell pkg-config --cflags --libs gtk+-3.0)
OBJS=movingavg.o

all: recording trainer master

tests: tests/all
	make -C tests

recording: $(OBJS) alsarec.o
	$(CC) $(CFLAGS) -orecording $(OBJS) alsarec.o

master: $(OBJS) master.o
	$(CC) $(CFLAGS) -omaster $(OBJS) master.o


trainer: $(OBJS) trainer.o
	$(CC) $(CFLAGS) -otrainer $(OBJS) trainer.o

clean:
	rm $(OBJS) trainer recording
