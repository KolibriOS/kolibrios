CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32  -Dstricmp=strcasecmp -DZLIB_SUPPORT -Dstrnicmp=strncasecmp -DUSE_SDL -DNDEBUG
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I. -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I $(SDK_DIR)/sources/zlib
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -Lsdl

SRC = fireworks.c main.c menu.c filter.c stub.c

OBJECTS = $(patsubst %.c, %.o, $(SRC))

default: $(patsubst %.c,%.o,$(SRC))
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem native -o jumpnbump $(OBJECTS) -ls -lSDLn -lsound -lz -lgcc -lc.dll 
	objcopy jumpnbump -O binary

%.o : %.c Makefile $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
