
LIBRARY = pixman-1

CC = gcc

CFLAGS = -c -O2 -mmmx -Winline

DEFINES = -DHAVE_CONFIG_H -DPACKAGE -DPIXMAN_NO_TLS -DUSE_MMX

INCLUDES = -I../pixman -I../newlib/include -I../newlib/include/sys

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

all:$(LIBRARY).a


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)

%.o: %.c $(SOURCES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<



 