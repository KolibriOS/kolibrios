LIBRARY= libdrm

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip
 
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -march=i686 -O2 -fomit-frame-pointer

LDFLAGS:= -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --version-script libdrm.ver --image-base=0
LDFLAGS+= --out-implib $(LIBRARY).dll.a

ARFLAGS = crs

INCLUDES= -I. -I./include/drm -I../newlib/libc/include

LIBPATH:= -L../../lib -L/home/autobuild/tools/win32/mingw32/lib

LIBS:=  -ldll -lc.dll

DEFINES= 

SOURCES = 	xf86drm.c			\
		intel/intel_bufmgr.c		\
		intel/intel_bufmgr_gem.c

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

# targets 

all:$(LIBRARY).a $(LIBRARY).dll


$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a ../../lib

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)
	mv -f $@ ../../bin
	mv -f $(LIBRARY).dll.a ../../lib   

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




