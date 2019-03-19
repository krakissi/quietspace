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
UTIL=../util
CC=gcc -L$(LIB) -I$(I) -s -O3 -Wall
ASSETS=assets_enc

all: $(O) $(BIN) $(LIB) $(LIB)/libkraknet.so $(BIN)/quietspace encoder

run: all
	@cd .. && ./init_nanogamo restart

stop:
	@cd .. && ./init_nanogamo stop

clean:
	rm -f $(LIB)/kraknet.so $(O)/*.o $(BIN)/quietspace $(LIB)/libkraknet.so $(UTIL)/encoder

encoder: $(UTIL)/encoder
	@echo "Base64 encoder OK."

# libkraknet (shared library of common functions)
$(LIB)/libkraknet.so: $(C)/generic.c
	$(CC) -c -fPIC -o $(O)/generic.o $(C)/generic.c
	$(CC) -shared -o $(LIB)/libkraknet.so $(O)/generic.o -lrt
	chmod a-x $(LIB)/libkraknet.so

# base64 asset encoding tool
$(UTIL)/encoder: $(C)/encoder.c $(C)/base64.h
	$(CC) -o $(UTIL)/encoder $(C)/encoder.c


# quiet space
$(BIN)/quietspace: $(C)/main.c $(O)/handler.o $(O)/qsfmt.o $(O)/dbpersistence.o $(O)/qsmap.o $(O)/loader.o $(LIB)/libkraknet.so
	$(CC) -o $(BIN)/quietspace $(C)/main.c $(O)/*.o -lkraknet -lmysqlclient

$(O)/dbpersistence.o: $(C)/dbpersistence.c $(C)/dbpersistence.h
	$(CC) -c $(C)/dbpersistence.c -o $(O)/dbpersistence.o

$(O)/handler.o: $(C)/handler.c $(I)/handler.h $(I)/generic.h $(C)/qsfmt.h $(C)/dbpersistence.h $(C)/qsgame.h $(C)/qsmap.h $(ASSETS)/*
	$(CC) -c $(C)/handler.c -o $(O)/handler.o

$(O)/qsfmt.o: $(C)/qsfmt.c $(C)/qsfmt.h
	$(CC) -c $(C)/qsfmt.c -o $(O)/qsfmt.o

$(O)/qsmap.o: $(C)/qsmap.c $(C)/qsmap.h
	$(CC) -c $(C)/qsmap.c -o $(O)/qsmap.o

$(O)/loader.o: $(C)/loader.c $(C)/loader.h
	$(CC) -c $(C)/loader.c -o $(O)/loader.o

# directories
$(O):
	mkdir $(O)

$(BIN):
	mkdir $(BIN)

$(LIB):
	mkdir $(LIB)
