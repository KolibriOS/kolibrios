# Makefile for dZ80 (with scripting support)

CC = gcc
BIND = gcc
RM = @rm -f

#   CFLAGS    flags for C compile
#   LFLAGS1   flags after output file spec, before obj file list
#   LFLAGS2   flags after obj file list (libraries, etc)

CFLAGS = -Ilua/include -Wall -O3
LFLAGS1 =
LFLAGS2 = -s

LUA_O = lua/src/lapi.o \
	lua/src/lcode.o \
	lua/src/ldebug.o \
	lua/src/ldo.o \
	lua/src/lfunc.o \
	lua/src/lgc.o \
	lua/src/llex.o \
	lua/src/lmem.o \
	lua/src/lobject.o \
	lua/src/lparser.o \
	lua/src/lstate.o \
	lua/src/lstring.o \
	lua/src/ltable.o \
	lua/src/ltests.o \
	lua/src/ltm.o \
	lua/src/lundump.o \
	lua/src/lvm.o \
	lua/src/lzio.o \
	lua/src/lib/lauxlib.o \
	lua/src/lib/lbaselib.o \
	lua/src/lib/lstrlib.o

DZ80_O = dz80.o config.o dissz80.o loadfile.o parsecmd.o script.o tables.o

dz80:	$(LUA_O) $(DZ80_O)
	$(BIND) $(LUA_O) $(DZ80_O) -o dz80 $(LFLAGS1) $(LFLAGS2)

clean:
	$(RM) $(DZ80_O)
	$(RM) $(LUA_O)
