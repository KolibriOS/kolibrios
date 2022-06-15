# Makefile for dZ80 without scripting support

CC = gcc
BIND = gcc
RM = @rm -f

#   CFLAGS    flags for C compile
#   LFLAGS1   flags after output file spec, before obj file list
#   LFLAGS2   flags after obj file list (libraries, etc)

CFLAGS = -D_DZ80_EXCLUDE_SCRIPT -Wall -O3
LFLAGS1 =
LFLAGS2 = -s

DZ80_O =  dz80.o dissz80.o loadfile.o parsecmd.o tables.o noscript.o

dz80ns: $(DZ80_O)
	$(BIND) $(DZ80_O) -o dz80ns $(LFLAGS1) $(LFLAGS2)

clean:
	$(RM) $(DZ80_O)

