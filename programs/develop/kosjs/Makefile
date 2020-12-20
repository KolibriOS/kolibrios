CC = kos32-gcc
LD = kos32-ld
OBJCOPY = objcopy
STRIP = strip

NAME = kosjs

SDK_DIR = ../../../contrib/sdk

LDFLAGS = -call_shared -nostdlib -T $(SDK_DIR)/sources/newlib/app-dynamic.lds --image-base 0
LIBPATH = -L$(SDK_DIR)/lib -L/home/autobuild/tools/win32/mingw32/lib -L libmujs
CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I libmujs

all:
	$(CC) $(CFLAGS) $(INCLUDES) kosjs.c import.c
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o $(NAME) $(NAME).o import.o -lmujs -lgcc -lc.dll 
	$(STRIP) -s $(NAME) -o $(NAME)
	$(OBJCOPY) $(NAME) -O binary

clean:
	rm -f $(NAME) *.o

