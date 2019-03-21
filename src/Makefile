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

pack: encoder
	@echo "Packing assets..."
	@$(UTIL)/pack.sh
	@echo "Done!"

stop:
	@cd .. && ./init_nanogamo stop

clean:
	rm -f $(LIB)/kraknet.so $(O)/*.o $(BIN)/quietspace $(LIB)/libkraknet.so $(BIN)/encoder

encoder: $(BIN)/encoder
	@

# libkraknet (shared library of common functions)
$(LIB)/libkraknet.so: $(C)/generic.c
	@echo "Building libkraknet.so..."
	@$(CC) -c -fPIC -o $(O)/generic.o $(C)/generic.c
	@$(CC) -shared -o $(LIB)/libkraknet.so $(O)/generic.o -lrt
	@chmod a-x $(LIB)/libkraknet.so

# base64 asset encoding tool
$(BIN)/encoder: $(C)/encoder.c $(C)/base64.h
	@echo "Building asset encoder..."
	@$(CC) -o $(BIN)/encoder $(C)/encoder.c
	@echo "Done!"


# quiet space
$(BIN)/quietspace: $(C)/main.c $(O)/loader.o $(O)/qsmap.o $(O)/qsfmt.o $(O)/dbpersistence.o $(O)/handler.o $(LIB)/libkraknet.so
	@echo "Building game binary..."
	@$(CC) -o $(BIN)/quietspace $(C)/main.c $(O)/*.o -lkraknet -lmysqlclient
	@echo "Done!"

$(O)/dbpersistence.o: $(C)/dbpersistence.c $(C)/dbpersistence.h
	@echo "Building database persistence..."
	@$(CC) -c $(C)/dbpersistence.c -o $(O)/dbpersistence.o

$(O)/handler.o: $(C)/handler.c $(I)/handler.h $(I)/generic.h $(C)/qsfmt.h $(C)/dbpersistence.h $(C)/qsgame.h $(C)/qsmap.h $(C)/loader.h
	@echo "Building net connection handler..."
	@$(CC) -c $(C)/handler.c -o $(O)/handler.o

$(O)/qsfmt.o: $(C)/qsfmt.c $(C)/qsfmt.h
	@echo "Building text formatting utils..."
	@$(CC) -c $(C)/qsfmt.c -o $(O)/qsfmt.o

$(O)/qsmap.o: $(C)/qsmap.c $(C)/qsmap.h
	@echo "Building map system..."
	@$(CC) -c $(C)/qsmap.c -o $(O)/qsmap.o

$(O)/loader.o: $(C)/loader.c $(C)/loader.h $(ASSETS)/*
	@echo "Building asset loader..."
	@$(CC) -c $(C)/loader.c -o $(O)/loader.o

# directories
$(O):
	@echo "Create object directory."
	@mkdir $(O)

$(BIN):
	@echo "Create binary directory."
	@mkdir $(BIN)

$(LIB):
	@echo "Create library directory."
	@mkdir $(LIB)
