
LIBRARY = pixman-1

CC = gcc

CFLAGS = -c -O2 -mmmx -Winline -fomit-frame-pointer

LDIMPORT:= -nostdlib --out-implib libpiximp.a --exclude-libs libamz.a
LDFLAGS:=  -shared -s -T ../newlib/dll.lds -Map map --image-base 0

DEFINES = -DHAVE_CONFIG_H -DPIXMAN_NO_TLS -D__DYNAMIC_REENT__ -DUSE_MMX

INCLUDES = -I../pixman -I../newlib/include

LIBPATH:= -L../newlib

LIBS:=  -lamz -lgcc -lcimp

SOURCES =				\
	pixman-image.c			\
	pixman-access.c			\
	pixman-access-accessors.c	\
	pixman-region16.c		\
	pixman-region32.c		\
	pixman-combine32.c		\
	pixman-combine64.c		\
	pixman-utils.c			\
	pixman-edge.c			\
	pixman-edge-accessors.c		\
	pixman-trap.c			\
	pixman-timer.c			\
	pixman-matrix.c			\
	pixman-gradient-walker.c	\
	pixman-linear-gradient.c	\
	pixman-radial-gradient.c	\
	pixman-bits-image.c		\
	pixman.c			\
	pixman-cpu.c			\
	pixman-fast-path.c		\
	pixman-implementation.c		\
	pixman-solid-fill.c		\
	pixman-general.c		\
	pixman-mmx.c			\
	$(NULL)
 
OBJECTS     = $(patsubst %.c, %.o, $(SOURCES))

# targets 

all:$(LIBRARY).a $(LIBRARY).dll


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)

$(LIBRARY).dll: $(OBJECTS) Makefile
	ld $(LDFLAGS) $(LDIMPORT) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS) 


%.o: %.c $(SOURCES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<



 