# Makefile for zlib
# Copyright (C) 1995-2010 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h

CC=gcc
CPP=gcc -E

CFLAGS= -O3 -c -fomit-frame-pointer

LDIMPORT:= -nostdlib --out-implib libzimp.a 

LDFLAGS:=  -shared -s -T ../newlib/dll.lds --image-base 0

INCLUDES= -I../newlib/include

LIBPATH:= -L../newlib

LIBS:=  -lamz -lgcc -lcimp

AR=ar rc

OBJC =	adler32.o compress.o crc32.o deflate.o		\
	gzclose.o gzlib.o gzread.o gzwrite.o		\
	infback.o inffast.o inflate.o inftrees.o	\
	trees.o uncompr.o zutil.o


# to use the asm code: make OBJA=match.o, PIC_OBJA=match.lo
OBJA =

OBJS = $(OBJC) $(OBJA)

all: libz.a zlib.dll

libz.a: $(OBJS)
	$(AR) $@ $(OBJS)

zlib.dll: $(OBJECTS) Makefile
#	dlltool -d zlib.def -Dlibz.dll -e exports.o -l libzimp.a
	ld $(LDFLAGS) $(LDIMPORT) $(LIBPATH) -o $@ $(OBJS) $(LIBS) zlib.def 


%.o: %.c Makefile
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<


