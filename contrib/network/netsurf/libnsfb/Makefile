# This is the makefile in kolibrios/contrib/network/netsurf/libcss
# Need to add recursive calling to all the subdirectories. For now, use the generated .a file for testing.

OUTFILE := $(CURDIR)/libnsfb.a

OBJS = $(wildcard src/libo.o)
OBJS := $(OBJS) $(wildcard src/plot/libo.o)
OBJS := $(OBJS) $(wildcard src/surface/libo.o)

include $(MENUETDEV)/makefiles/Makefile_for_lib
