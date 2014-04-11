# This is the makefile in kolibrios/contrib/network/netsurf/libcss
# Need to add recursive calling to all the subdirectories. For now, use the generated .a file for testing.

OUTFILE := $(CURDIR)/libcss.a

OBJS = $(wildcard src/libo.o)
OBJS := $(OBJS) $(wildcard src/charset/libo.o)
OBJS := $(OBJS) $(wildcard src/lex/libo.o)
OBJS := $(OBJS) $(wildcard src/parse/properties/libo.o)
OBJS := $(OBJS) $(wildcard src/parse/libo.o)
OBJS := $(OBJS) $(wildcard src/select/properties/libo.o)
OBJS := $(OBJS) $(wildcard src/select/libo.o)
OBJS := $(OBJS) $(wildcard src/utils/libo.o)

include $(MENUETDEV)/makefiles/Makefile_for_lib
