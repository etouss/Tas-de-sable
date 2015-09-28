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


ifeq ($(ONSUME),yes)
OFLAGS := $(OFLAGS) -O4
DEFINES := $(DEFINES) -DNDEBUG
CFLAGS := $(CFLAGS) -std=$(STDC) -pedantic -Wno-format-zero-length
else #not on sume
DEFINES := $(DEFINES)
CFLAGS := $(CFLAGS) -std=$(STDC) -pedantic
endif

INSTALLDIR = $(HOME)/bin

c-sources = display.c options.c sand.c time.c trace.c libphs.c

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

display.o: trace.h sand.h
options.o: sand.h version.h
sand.o: trace.h sand.h
time.o: sand.h
trace.o: trace.h
