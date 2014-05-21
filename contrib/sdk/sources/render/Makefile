
LIBRARY= librender

CC=gcc
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

AR= ar

INCLUDES= -I. -I../newlib/libc/include -I../Mesa/include -I../libdrm/include/drm

LIBPATH:= -L../../lib


DEFINES= -D__unix__ -DMESA_EGL_NO_X11_HEADERS


SOURCES = render.c	\
	  blit.c	\
	  swap.c	
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


# targets 


all:$(LIBRARY).a

$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a ../../lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




