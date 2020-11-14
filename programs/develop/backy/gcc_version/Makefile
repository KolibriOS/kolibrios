CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../../contrib/sdk)
LDFLAGS = -call_shared -nostdlib -T $(SDK_DIR)/sources/newlib/app-dynamic.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

ifndef LANG_backy
LANG_backy = lang_en
endif

default: backy

backy: $(OBJECTS) Makefile
	echo "#define $(LANG_backy)" > Backy_lang.h
	$(CC) $(CFLAGS) $(INCLUDES) -o Backy.o Backy.c
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem console -o Backy Backy.o -lgcc -lc.dll
	kos32-strip -s Backy -o Backy
	objcopy Backy -O binary
	rm Backy.o
	rm Backy_lang.h
clean:
	rm Backy
