
LIBRARY = pixman-1

CC = gcc
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -Wall -Winline -fomit-frame-pointer

LD = ld
LDFLAGS = -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --image-base=0 --out-implib lib$(LIBRARY).dll.a

STRIP = $(PREFIX)strip

INCLUDES= -I. -I../newlib/include

LIBPATH:= -L../newlib

LIBS:=  -ldll -lc.dll -lgcc

#DEFINES = -DHAVE_CONFIG_H -DPIXMAN_NO_TLS
DEFINES = -DHAVE_CONFIG_H


SOURCES =				\
	pixman.c			\
	pixman-access.c			\
	pixman-access-accessors.c	\
	pixman-bits-image.c		\
	pixman-combine32.c		\
	pixman-combine-float.c		\
	pixman-conical-gradient.c	\
	pixman-edge.c			\
	pixman-edge-accessors.c		\
	pixman-fast-path.c		\
	pixman-filter.c			\
	pixman-general.c		\
	pixman-glyph.c			\
	pixman-gradient-walker.c	\
	pixman-image.c			\
	pixman-implementation.c		\
	pixman-linear-gradient.c	\
	pixman-matrix.c			\
	pixman-noop.c			\
	pixman-radial-gradient.c	\
	pixman-region16.c		\
	pixman-region32.c		\
	pixman-solid-fill.c		\
	pixman-timer.c			\
	pixman-trap.c			\
	pixman-utils.c			\
	pixman-x86.c			\
	pixman-mmx.c			\
	pixman-sse2.c			\
	$(NULL)
 
OBJECTS     = $(patsubst %.c, %.o, $(SOURCES))

# targets 

all:$(LIBRARY).a $(LIBRARY).dll


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)

$(LIBRARY).dll: $(LIBRARY).def $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(LIBRARY).def $(OBJECTS) $(LIBS)
	$(STRIP) $@
	sed -f ../newlib/cmd1.sed $(LIBRARY).def > mem
	sed -f ../newlib/cmd2.sed mem >$(LIBRARY).inc

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

pixman-mmx.o: pixman-mmx.c Makefile
	$(CC) $(CFLAGS) -mmmx $(DEFINES) $(INCLUDES) -o $@ $<

pixman-sse2.o: pixman-sse2.c Makefile
	$(CC) $(CFLAGS) -msse2 $(DEFINES) $(INCLUDES) -o $@ $<


clean: 
	-rm -f *.o 
 