CFLAGS += -lncurses -Wall -O3 -Wextra
CC = gcc

all: cursedle

cursedle: cursedle.c words.h Makefile
	$(CC) -o $@ $@.c $(CFLAGS)

.PHONY: all install uninstall clean
