.PHONY: all clean

CC = gcc
CFLAGS = -Wall -g -w


all: mync ttt


mync: mync.c
	$(CC) $(CFLAGS) -o $@ $^


ttt: ttt.c
	$(CC) $(CFLAGS)  -o $@ $^



clean: 
	rm -f mync ttt 
