# File: Makefile pour SANDPILE
#

all : sand

CC := gcc
STDC := c99 #was ansi

THISDIR := $(shell pwd)
DEFINES = #-DTRACE
OFLAGS =
LDLIBS = -lncurses
LOADLIBES = #deprecated, defined here to avoid the annoying warning message
LDFLAGS = -L.
CFLAGS = $(DEFINES) -I. $(OFLAGS) -Wall -W

DEFINES := $(DEFINES)
CFLAGS := $(CFLAGS) -std=$(STDC) -pedantic -Wno-deprecated-declarations #-Wno-unused-variable #in beta versions

INSTALLDIR = $(HOME)/bin

c-sources = board.c display.c memmat.c options.c sand.c system.c time.c trace.c libphs.c

h-files = sand.h trace.h version.h
o-files = $(c-sources:.c=.o) 

tagsfiles = $(c-sources) $(h-files)

# recompile all when Makefile touched
$(o-files) : Makefile version.h

sand : $(o-files)

# MAKEFILE
TAGS : $(tagsfiles)
	etags $^

clean :
	rm -f core texput.log *.o 

zip :
	zip phs_sand.zip *.c *.h Makefile

# DO NOT DELETE, UPDATE WITH "MAKE DEPEND"

board.o: sand.h version.h trace.h
display.o: sand.h version.h trace.h
memmat.o: sand.h version.h trace.h
options.o: sand.h version.h trace.h
sand.o: sand.h version.h trace.h
system.o: sand.h version.h trace.h
time.o: sand.h version.h trace.h
trace.o: trace.h
