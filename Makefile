.PHONY: all clean

CC = gcc
CFLAGS = -Wall -g
COV = -fprofile-arcs -ftest-coverage

all: mync ttt


mync: mync.c
	$(CC) $(CFLAGS) $(COV) -o $@ $^


ttt: ttt.c
	$(CC) $(CFLAGS) $(COV) -o $@ $^



clean: 
	rm -f mync ttt *.gcno *.gcda *.gcov