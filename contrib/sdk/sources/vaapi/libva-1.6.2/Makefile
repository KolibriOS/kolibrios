LIBRARY = libva

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/lib

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -Wall -Winline -fno-ident
CFLAGS+=  -mno-ms-bitfields -fomit-frame-pointer -Wno-attributes -Wno-unused

LDFLAGS = -shared -s -T dll.lds --entry _DllStartup --image-base=0
LDFLAGS+= --out-implib $(LIBRARY).dll.a

ARFLAGS = crs

INCLUDES= -I../../newlib/libc/include -I../../libdrm -I../../libdrm/intel -I../../libdrm/include/drm
INCLUDES+= -I. -I./va 

LIBS:=  -ldll -ldrm.dll -lgcc -lc.dll

DEFINES:= -DHAVE_CONFIG_H

SOURCES =				\
	va/drm/va_drm.c			\
	va/drm/va_drm_utils.c		\
	va/va.c				\
	va/va_compat.c			\
	va/va_fool.c			\
	va/va_tpi.c			\
	va/va_trace.c

 
OBJECTS     = $(patsubst %.c, %.o, $(SOURCES))

# targets 

all: $(LIBRARY).dll

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)
	$(STRIP) $@
	mv -f $@ ../../../bin
	mv -f $(LIBRARY).dll.a $(INSTALLDIR)
	

%.o : %.c Makefile
	$(CC) $(INCLUDES) $(CFLAGS) $(DEFINES)  -o $@ $<


clean: 
	-rm -f *.o 
 