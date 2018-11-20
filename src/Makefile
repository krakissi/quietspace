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

run: all
	@cd .. && ./init_nanogamo restart

stop:
	@cd .. && ./init_nanogamo stop

clean:
	rm -f $(LIB)/kraknet.so $(O)/*.o $(BIN)/quietspace $(LIB)/libkraknet.so

# libkraknet (shared library of common functions)
$(LIB)/libkraknet.so: $(C)/generic.c
	$(CC) -c -fPIC -o $(O)/generic.o $(C)/generic.c
	$(CC) -shared -o $(LIB)/libkraknet.so $(O)/generic.o -lrt
	chmod a-x $(LIB)/libkraknet.so


# quiet space
$(BIN)/quietspace: $(C)/main.c $(O)/handler.o $(O)/qsfmt.o $(O)/dbpersistence.o $(O)/qsmap.o $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/quietspace $(C)/main.c $(O)/*.o -lkraknet -lmysqlclient

$(O)/dbpersistence.o: $(C)/dbpersistence.c $(C)/dbpersistence.h
	$(CC) -c $(C)/dbpersistence.c -o $(O)/dbpersistence.o

$(O)/handler.o: $(C)/handler.c $(I)/handler.h $(I)/generic.h $(C)/qsfmt.h $(C)/dbpersistence.h $(C)/qsgame.h $(C)/qsmap.h
	$(CC) -c $(C)/handler.c -o $(O)/handler.o

$(O)/qsfmt.o: $(C)/qsfmt.c $(C)/qsfmt.h
	$(CC) -c $(C)/qsfmt.c -o $(O)/qsfmt.o

$(O)/qsmap.o: $(C)/qsmap.c $(C)/qsmap.h
	$(CC) -c $(C)/qsmap.c -o $(O)/qsmap.o

# directories
$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

$(LIB):
	mkdir $(LIB)
