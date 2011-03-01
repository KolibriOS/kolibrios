# Makefile for zlib
# Copyright (C) 1995-2010 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h


CC=gcc
CPP=gcc -E

CFLAGS= -O3 -c -fomit-frame-pointer

INCLUDES= -I../newlib/include


STATICLIB=libz.a
LIBS=$(STATICLIB) $(SHAREDLIBV)

AR=ar rc

OBJC =	adler32.o compress.o crc32.o deflate.o		\
	gzclose.o gzlib.o gzread.o gzwrite.o		\
	infback.o inffast.o inflate.o inftrees.o	\
	trees.o uncompr.o zutil.o


# to use the asm code: make OBJA=match.o, PIC_OBJA=match.lo
OBJA =

OBJS = $(OBJC) $(OBJA)

all: libz.a

libz.a: $(OBJS)
	$(AR) $@ $(OBJS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<


