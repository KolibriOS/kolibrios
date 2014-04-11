# This is the makefile in kolibrios/contrib/network/netsurf/libhubbub
# Need to add recursive calling to all the subdirectories. For now, use the generated .a file for testing.

OUTFILE := $(CURDIR)/libhubbub.a

OBJS = $(wildcard src/*.o)
OBJS := $(OBJS) $(wildcard src/charset/*.o)
OBJS := $(OBJS) $(wildcard src/tokeniser/*.o)
OBJS := $(OBJS) $(wildcard src/treebuilder/*.o)
OBJS := $(OBJS) $(wildcard src/utils/*.o)

include $(MENUETDEV)/makefiles/Makefile_for_lib
