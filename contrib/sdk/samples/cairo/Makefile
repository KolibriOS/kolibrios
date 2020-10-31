CC = kos32-gcc
LD = kos32-ld 

SDK_DIR:= $(abspath ../..)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/cairo/src -I $(SDK_DIR)/sources/freetype/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

SOURCES = cairo.c	\
	  $(NULL)
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

default: cairo

cairo: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -Map cairo.map -o cairo $(OBJECTS) -lcairo2.dll -lgcc -lc.dll
	objcopy cairo -O binary

%.o : %.c Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
