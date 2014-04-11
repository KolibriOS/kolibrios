# This is the makefile in kolibrios/contrib/network/netsurf/libdom/src
# Need to add recursive calling to all the subdirectories. For now, use the generated .a file for testing.

OUTFILE := $(CURDIR)/libdom.a

OBJS = $(wildcard core/libo.o)
OBJS := $(OBJS) $(wildcard events/libo.o)
OBJS := $(OBJS) $(wildcard html/libo.o)
OBJS := $(OBJS) $(wildcard utils/libo.o)
OBJS := $(OBJS) $(wildcard ../bindings/hubbub/libo.o)

include $(MENUETDEV)/makefiles/Makefile_for_lib

# Contents before above changes were added:
# Src 
#NSBUILD ?= /usr/share/netsurf-buildsystem/makefiles
#include $(NSBUILD)/Makefile.subdir
