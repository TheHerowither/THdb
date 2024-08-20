CFLAGS=-Wall -Wextra



all: dbserver bin

dbserver: dbserver.c bin
	gcc dbserver.c $(CFLAGS) -o bin/dbserver -lpthread

bin:
	mkdir -p bin
