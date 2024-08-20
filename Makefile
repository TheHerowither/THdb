CFLAGS=-Wall -Wextra



all: dbserver bin

dbserver: dbserver.c bin
	gcc dbserver.c $(CFLAGS) -o bin/dbserver

bin:
	mkdir -p bin
