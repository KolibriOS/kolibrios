LIBRARY= libdrm

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/lib

INCLUDES= -I. -I./include/drm -I../newlib/libc/include
 
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -D_POSIX_TIMERS -D_POSIX_MONOTONIC_CLOCK
CFLAGS+=  -c -march=i686 -O2 -fno-ident -fomit-frame-pointer

LDFLAGS:= -shared -s -T dll.lds --entry _DllStartup --version-script libdrm.ver --image-base=0
LDFLAGS+= --out-implib $(LIBRARY).dll.a

ARFLAGS = crs

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
	mv -f $(LIBRARY).a $(INSTALLDIR)

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)
	mv -f $@ ../../bin
	mv -f $(LIBRARY).dll.a $(INSTALLDIR)

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




