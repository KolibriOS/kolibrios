CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../contrib/sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

default:
	$(CC) $(CFLAGS) $(INCLUDES) getline.c
	$(CC) $(CFLAGS) $(INCLUDES) -DGCC_BUILD tte.c
	$(CC) $(CFLAGS) $(INCLUDES) notify.c
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem native -o tte getline.o tte.o notify.o -lgcc -lc.dll
	objcopy tte -O binary

TCC:
	kos32-tcc -DTCC_BUILD tte.c getline.c notify.c -lck -o tte -I ../../develop/ktcc/trunk/libc/include

clean:
	rm *.o
