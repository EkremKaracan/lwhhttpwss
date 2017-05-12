VERSION=1.0
CC=gcc
CFLAGS=-O3 -Wall -I../lib -Wno-unused-variable -Wno-unused-result -Wno-pointer-sign -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
BIN=../../bin/mkcache

CORE=../lib/utils.o ../lib/version.o
SRC= mkcache_main.o ../lib/cache.o ../lib/foreachfile.o ../lib/mimetypes.c  $(CORE) 
LIBS=-lz -lpthread -ldl

%.o: %.c
	$(CC) $(CFLAGS) -c $? -o $@

all: $(SRC) 
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LIBS)
	
clean:
	rm -vfr *~ *.o $(BIN) $(SRC)
