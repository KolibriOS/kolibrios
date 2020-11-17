CC = kos32-gcc
LD = kos32-ld

SDK_DIR = ../../../
LDFLAGS = -nostdlib -static --image-base 0 -T $(SDK_DIR)/sources/newlib/app.lds

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I ../include/
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

default: fire

fire: $(OBJECTS) Makefile
	$(CC) $(CFLAGS) $(INCLUDES) -o sdltest.o sdltest.c
	$(CC) $(CFLAGS) $(INCLUDES) -o testbitmap.o testbitmap.c
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o sdltest sdltest.o -lgcc -lSDL -lc.dll -lc -lsound
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o testbitmap testbitmap.o -lgcc -lSDL -lc.dll -lc -lsound
	kos32-strip -s sdltest -o sdltest
	kos32-strip -s testbitmap -o testbitmap
	objcopy testbitmap -O binary
	objcopy sdltest -O binary
	rm testbitmap.o
	rm sdltest.o
clean:
	rm testbitmap
	rm sdltest
