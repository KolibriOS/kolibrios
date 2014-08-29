
FASM= fasm.exe
CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer

ARFLAG = crs

SDK_DIR:= $(abspath ../../..)
LIB_DIR:= $(SDK_DIR)/lib

DEFINES = -D__unix__ -DMESA_EGL_NO_X11_HEADERS

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/Mesa/include -I$(SDK_DIR)/sources/eglut

LIBS_EGL:= -leglut -legl.dll
LIBS_OSM:= -losmesa.dll
LIBS:= -lGL.dll -lpixlib.dll -lgcc -lc.dll -lapp

LIBPATH:= -L$(LIB_DIR) -L/home/autobuild/tools/win32/mingw32/lib

LDFLAGS = -static -nostdlib --stack 0x200000 -T$(SDK_DIR)/sources/newlib/app.lds --image-base 0

SOURCES = 	gears.c				\
		eglgears.c			\
		osgears.c
 
OBJECTS =  $(patsubst %.asm, %.o, $(patsubst %.c, %.o, $(SOURCES)))


# targets 

all: demos

demos: eglgears osgears

eglgears: gears.o eglgears.o Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ gears.o eglgears.o $(LIBS_EGL) $(LIBS)
	kos32-objcopy $@ -O binary

osgears: gears.o osgears.o Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ gears.o osgears.o $(LIBS_OSM) $(LIBS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




