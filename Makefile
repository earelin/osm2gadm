export CC = gcc
export CCFLAGS = -g -Wall -O0 --pedantic --std=c99
export PKGCONFIG = pkg-config
export CCHEADERS = $(shell $(PKGCONFIG) --cflags glib-2.0 libpq)
export LDLIBS = $(shell $(PKGCONFIG) --libs glib-2.0 libpq) -lgeos_c
export DOXYGEN = doxygen

all: osm2gadm
	
osm2gadm: main.o database.o process.o datatypes.o utils.o
	$(CC) $(CCFLAGS) $(CCHEADERS) -o $@ $+ $(LDLIBS)

main.o: main.c database.h datatypes.h
	$(CC) -c $(CCFLAGS) $(CCHEADERS) main.c $(LDLIBS)
	
database.o: database.c database.h datatypes.h
	$(CC) -c $(CCFLAGS) $(CCHEADERS) database.c $(LDLIBS)
	
process.o: process.c process.h
	$(CC) -c $(CCFLAGS) $(CCHEADERS) process.c $(LDLIBS)
	
datatypes.o: datatypes.c datatypes.h
	$(CC) -c $(CCFLAGS) $(CCHEADERS) datatypes.c $(LDLIBS)
	
utils.o: utils.c utils.h
	$(CC) -c $(CCFLAGS) $(CCHEADERS) utils.c $(LDLIBS)
	
clean:
	-rm osm2gadm *.o

.PHONY: all clean
