
LIBRARY= libdrm

CC=gcc
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

LD = ld

AR= ar

STRIP = $(PREFIX)strip

LDFLAGS:=  -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --version-script libdrm.ver --image-base=0 --out-implib $(LIBRARY).dll.a

INCLUDES= -I. -I./include/drm -I../newlib/include

LIBPATH:= -L../../lib

LIBS:=  -ldll -lc.dll


DEFINES= 


SOURCES = 	xf86drm.c			\
		intel/intel_bufmgr.c		\
		intel/intel_bufmgr_gem.c
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


# targets 


all:$(LIBRARY).a $(LIBRARY).dll


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a ../../lib

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)
	$(STRIP) $@
	mv -f $@ ../../bin
	mv -f $(LIBRARY).dll.a ../../lib   

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




