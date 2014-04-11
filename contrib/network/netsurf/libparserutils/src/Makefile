
#include $(NSBUILD)/Makefile.subdir
# This is the makefile in kolibrios/contrib/network/netsurf/libparserutils/src
# Need to add recursive calling to all the subdirectories. For now, use the generated .a file for testing.

OUTFILE := $(CURDIR)/libparseutils.a
OBJS = $(wildcard charset/*.o)
OBJS := $(OBJS) $(wildcard charset/codecs/*.o)
OBJS := $(OBJS) $(wildcard charset/encodings/*.o)
OBJS := $(OBJS) $(wildcard input/*.o)
OBJS := $(OBJS) $(wildcard utils/*.o)

include $(MENUETDEV)/makefiles/Makefile_for_lib

