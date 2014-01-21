
LIBRARY= libeglut

CC=gcc
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

AR= ar

INCLUDES= -I. -I../newlib/include -I../Mesa/include -I../Mesa/src/gbm/main -I../render

LIBPATH:= -L../../lib


DEFINES= -D__unix__ -DMESA_EGL_NO_X11_HEADERS


SOURCES = eglut.c	\
	  eglut_screen.c
 

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




