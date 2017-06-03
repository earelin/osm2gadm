export CC = gcc
export CCFLAGS = -g -Wall -O0 --pedantic --std=c99
export PKGCONFIG = pkg-config
export CCHEADERS = $(shell $(PKGCONFIG) --cflags glib-2.0 gio-2.0 libpq)
export LDLIBS = $(shell $(PKGCONFIG) --libs glib-2.0 gio-2.0 libpq) -lgeos_c
export GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)
export DOXYGEN = doxygen

all: osm2gadm
	
osm2gadm: main.o database.o process.o datatypes.o utils.o resources/gresources.o
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
	
resources/gresources.o: resources/gresources.c
	$(CC) -c $(CCFLAGS) $(CCHEADERS) -o $@ resources/gresources.c  $(LDLIBS)

resources/gresources.c: resources/gresources.xml resources/create_tables.sql
	$(GLIB_COMPILE_RESOURCES) resources/gresources.xml --target=$@ --sourcedir=resources --generate-source
	
clean:
	-rm osm2gadm *.o *~ resources/gresources.c

.PHONY: all clean
