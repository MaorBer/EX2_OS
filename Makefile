.PHONY: all clean

CC = gcc
CFLAGS = -Wall -g


all: mync ttt


mync: mync.c
	$(CC) $(CFLAGS)  -o $@ $^


ttt: ttt.c
	$(CC) $(CFLAGS)  -o $@ $^



clean: 
	rm -f mync ttt 