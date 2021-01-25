CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I.
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib


SRC = $(notdir $(wildcard *.c))

OBJECTS = $(patsubst %.c, %.o, $(SRC))

default: $(patsubst %.c,%.o,$(SRC))
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem native -o doom $(OBJECTS) -lSDLn -lsound -lgcc -lc.dll
	objcopy doom -O binary

%.o : %.c Makefile $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
