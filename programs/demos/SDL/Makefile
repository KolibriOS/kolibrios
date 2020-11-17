CC = kos32-gcc
LD = kos32-ld

SDK_DIR = ../../../contrib/sdk
LDFLAGS = -nostdlib -static --image-base 0 -T $(SDK_DIR)/sources/newlib/app.lds

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/SDL-1.2.2_newlib/include/
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

default: SDL-demo

SDL-demo: $(OBJECTS) Makefile
	$(CC) $(CFLAGS) $(INCLUDES) -o SDLTest.o SDLTest.c
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o SDLTest SDLTest.o -lgcc -lSDL -lc.dll -lc -lsound
	kos32-strip -s SDLTest -o SDLTest
	objcopy SDLTest -O binary
	rm SDLTest.o
clean:
	rm SDLTest
