
CC=gcc
CFLAGS=-Wall -std=c99 -D_POSIX_C_SOURCE -I. -I/usr/include/alsa -lasound
OBJS=movingavg.o alsarec.o

all: recording

tests: tests/all
	make -C tests

recording: $(OBJS)
	$(CC) $(CFLAGS) -orecording $(OBJS)



