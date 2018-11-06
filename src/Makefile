# quiet space
# mperron (2018)
#
# To build:
# 	$ make
#
# To clean (remove compiled binaries):
# 	$ make clean

I=.
C=.
O=o
BIN=../bin
LIB=../lib
CC=gcc -L$(LIB) -I$(I) -s -O3 -Wall

all: $(O) $(BIN) $(LIB) $(LIB)/libkraknet.so $(BIN)/quietspace

clean:
	rm -f $(LIB)/kraknet.so $(O)/*.o $(BIN)/quietspace $(LIB)/libkraknet.so

# libkraknet (shared library of common functions)
$(LIB)/libkraknet.so: $(C)/generic.c
	$(CC) -c -fPIC -o $(O)/generic.o $(C)/generic.c
	$(CC) -shared -o $(LIB)/libkraknet.so $(O)/generic.o -lrt
	chmod a-x $(LIB)/libkraknet.so


# quiet space
$(BIN)/quietspace: $(C)/main.c $(O)/handler.o $(LIB)/libkraknet.so
	$(CC) -Wall -o $(BIN)/quietspace $(C)/main.c $(O)/handler.o -lkraknet -lmysqlclient

$(O)/handler.o: $(C)/handler.c $(I)/handler.h $(I)/generic.h
	$(CC) -Wall -c $(C)/handler.c -o $(O)/handler.o

# directories
$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

$(LIB):
	mkdir $(LIB)
