
LIBRARY= libeglut
IMPLIB=  libeglut.dll.a

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

INCLUDES= -I. -I../newlib/libc/include -I../Mesa/include -I../Mesa/src/gbm/main -I../libdrm/include

LDFLAGS:= -shared -s -nostdlib -T ../newlib/libc/dll.lds --entry _DllStartup --image-base=0
LDFLAGS+= --out-implib $(IMPLIB)

LIBPATH:= -L../../lib
LIBS:=  -ldll -legl.dll -lGL.dll -lc.dll

DEFINES= -D__unix__ -DMESA_EGL_NO_X11_HEADERS


SOURCES = eglut.c	\
	  eglut_screen.c\
	  render.c	\
	  swap.c	
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


# targets 


all:$(LIBRARY).a $(LIBRARY).dll

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) crs $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a ../../lib

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS) 
	mv -f $(LIBRARY).dll ../../bin
	mv -f $(IMPLIB) ../../lib   

%.o : %.c Makefile eglut.h eglutint.h render.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




