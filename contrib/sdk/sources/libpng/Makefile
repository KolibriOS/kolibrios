LIBRARY= libpng16

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

CFLAGS = -UWIN32 -UWindows -U_WINDOWS -U_WIN32 -U__WIN32__ -c -O2 -fomit-frame-pointer

LDFLAGS= -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --image-base=0
LDFLAGS+=  --out-implib $(LIBRARY).dll.a
 
ARFLAGS:= crs

INCLUDES= -I. -I../newlib/include -I../zlib

LIBPATH:= -L../../lib

LIBS:=  -ldll -lc.dll -lz.dll

DEFS = -DHAVE_CONFIG_H

LIBPNG_DEFINES = -DPNG_CONFIGURE_LIBPNG 

DEFINES= $(DEFS) $(LIBPNG_DEFINES)

ifeq ($(findstring ebox,$(MAKECMDGOALS)),ebox)
CFLAGS+=-march=pentium-mmx
endif


SOURCES = png.c pngerror.c pngget.c pngmem.c pngpread.c	\
	  pngread.c pngrio.c pngrtran.c pngrutil.c	\
	  pngset.c pngtrans.c pngwio.c pngwrite.c 	\
	  pngwtran.c pngwutil.c
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


# targets 


all:$(LIBRARY).a $(LIBRARY).dll

ebox:$(LIBRARY).a $(LIBRARY).dll

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a ../../lib

$(LIBRARY).dll: libpng16.def $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@  libpng16.def $(OBJECTS) $(LIBS)
	$(STRIP) $@
	#sed -f ../newlib/cmd1.sed libpng16.def > mem
	#sed -f ../newlib/cmd2.sed mem >libpng.inc
	mv -f $@ ../../bin
	mv -f $(LIBRARY).dll.a ../../lib   


%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




