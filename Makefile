
CC=gcc
CFLAGS=-g -Wall -std=c99 -D_POSIX_C_SOURCE -I. -I/usr/include/alsa -lm -lasound $(shell pkg-config --cflags --libs gtk+-3.0)
OBJS=movingavg.o

all: recording trainer master sigavg

tests: tests/all
	make -C tests

recording: $(OBJS) alsarec.o
	$(CC) $(CFLAGS) -orecording $(OBJS) alsarec.o

master: $(OBJS) master.o
	$(CC) $(CFLAGS) -omaster $(OBJS) master.o

sigavg: $(OBJS) sigavg.o
	$(CC) $(CFLAGS) -osigavg $(OBJS) sigavg.o

trainer: $(OBJS) trainer.o
	$(CC) $(CFLAGS) -otrainer $(OBJS) trainer.o

clean:
	rm $(OBJS) trainer recording master sigavg
