include $(MENUETDEV)/osrules.mak
OUTFILE = libmcoff.a
OBJS = loadcoff.o section.o symtab.o relocate.o debug.o
CFLAGS = -O2 -fomit-frame-pointer -fno-common -DMCOFF_MENUETOS=1

_all: $(OUTFILE)
ifdef ON_MINGW
	del $(MENUETDEV)\lib\$(OUTFILE)
	copy $(OUTFILE) $(MENUETDEV)\lib
	copy *.h $(MENUETDEV)\include\mcoff
else
	rm -f $(MENUETDEV)/lib/$(OUTFILE)
	cp $(OUTFILE) $(MENUETDEV)/lib
	cp *.h $(MENUETDEV)/include/mcoff
endif

include $(MENUETDEV)/makefiles/Makefile_for_lib
