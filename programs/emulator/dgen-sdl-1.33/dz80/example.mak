# Makefile for dZ80 Example (without scripting support)

CC = gcc
BIND = gcc
RM = @rm -f

#   CFLAGS    flags for C compile
#   LFLAGS1   flags after output file spec, before obj file list
#   LFLAGS2   flags after obj file list (libraries, etc)

CFLAGS = -D_DZ80_EXCLUDE_SCRIPT -Wall 
LFLAGS1 =
LFLAGS2 = -s

DZ80_O =  example.o dissz80.o loadfile.o tables.o noscript.o

example: $(DZ80_O)
	$(BIND) $(DZ80_O) -o example $(LFLAGS1) $(LFLAGS2)

clean:
	$(RM) $(DZ80_O)

