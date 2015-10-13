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

c-sources := $(shell ls *.c)
h-files := $(shell ls *.h)
o-files := $(c-sources:.c=.o) 

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

depend :
	makedepend -DMAKEDEPEND_IGNORE -- $(c-sources)

# DO NOT DELETE, UPDATE WITH "MAKE DEPEND"

board.o: sand.h trace.h utils.h
display.o: sand.h trace.h
libphs.o: print.h bool.h
memmat.o: sand.h trace.h
options.o: sand.h trace.h utils.h version.h
report.o: sand.h trace.h version.h
sand.o: sand.h trace.h version.h
system.o: sand.h trace.h
time.o: sand.h trace.h
trace.o: trace.h
utils.o: sand.h trace.h utils.h
