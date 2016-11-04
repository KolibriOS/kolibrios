LIBRARY= pixlib3

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/lib

ARFLAGS = crs
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer
LDFLAGS:= -shared -s -Map px.map -T dll.lds --entry _DllStartup --image-base=0
LDFLAGS+= --file-alignment 256

INCLUDES= -I. -I../newlib/libc/include 
 
PXGL_INCLUDES= $(INCLUDES) -I../libdrm/intel -I../libdrm/include
PXGL_INCLUDES+= -I ../Mesa/mesa-9.2.5/include -I../Mesa/mesa-9.2.5/src/gbm/main
 
LIBS:=  -ldll -legl.dll -lGL.dll -lgcc -lc.dll
#LIBS+= -ldrm.dll

DEFINES:= -DNDEBUG
PXGL_DEFINES:= $(DEFINES) -D__unix__ -DMESA_EGL_NO_X11_HEADERS

SRC_PIXLIB = pixlib3.c
SRC_PXGL   = egl.c pxgl.c

OBJ_PIXLIB =  $(patsubst %.c, %.o, $(SRC_PIXLIB))
OBJ_PXGL =   $(patsubst %.c, %.o, $(SRC_PXGL))
 
# targets 

all: lib$(LIBRARY).a pixlib-gl.dll

lib$(LIBRARY).a: $(OBJ_PIXLIB) Makefile
	$(AR) $(ARFLAGS) lib$(LIBRARY).a $(OBJ_PIXLIB)
	mv -f lib$(LIBRARY).a $(INSTALLDIR)

pixlib-gl.dll: $(OBJ_PXGL) Makefile
	$(LD) $(LDFLAGS) $(PXFLAGS) -o $@ $(OBJ_PXGL) $(LIBS)
	mv -f $@ ../../bin

pixlib3.o : pixlib3.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(PXGL_DEFINES) $(PXGL_INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




