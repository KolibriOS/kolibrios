
LIBRARY= libpng

CC=gcc
CPP=gcc -E
AR= ar rc

CFLAGS = -c -O2 -fomit-frame-pointer

DEFS = -DHAVE_CONFIG_H

LIBPNG_DEFINES = -DPNG_CONFIGURE_LIBPNG 

DEFINES= $(DEFS) $(LIBPNG_DEFINES)

INCLUDES= -I../newlib/include -I../zlib


SOURCES = png.c pngerror.c pngget.c pngmem.c pngpread.c	\
	  pngread.c pngrio.c pngrtran.c pngrutil.c	\
	  pngset.c pngtrans.c pngwio.c pngwrite.c 	\
	  pngwtran.c pngwutil.c png.h pngconf.h 	\
	  pngdebug.h pnginfo.h pngpriv.h pngstruct.h
 


OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


# targets 


all:$(LIBRARY).a


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)


%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<



