
LIBRARY= libpng

CC=gcc
CPP=gcc -E
AR= ar rc

CFLAGS = -c -O2 -fomit-frame-pointer

LDIMPORT:= -nostdlib --out-implib libpngimp.a 

LDFLAGS:=  -shared -s -T../newlib/dll.lds --image-base 0

DEFS = -DHAVE_CONFIG_H

LIBPNG_DEFINES = -DPNG_CONFIGURE_LIBPNG 

DEFINES= $(DEFS) $(LIBPNG_DEFINES)

INCLUDES= -I../newlib/include -I../zlib

LIBPATH:= -L../newlib -L../zlib

LIBS:=  -lamz -lgcc -lzimp -lcimp 

SOURCES = png.c pngerror.c pngget.c pngmem.c pngpread.c	\
	  pngread.c pngrio.c pngrtran.c pngrutil.c	\
	  pngset.c pngtrans.c pngwio.c pngwrite.c 	\
	  pngwtran.c pngwutil.c
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


# targets 


all:$(LIBRARY).a $(LIBRARY).dll


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)

$(LIBRARY).dll: $(OBJECTS) Makefile
	ld $(LDFLAGS) $(LDIMPORT) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS) symbols.def 

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




