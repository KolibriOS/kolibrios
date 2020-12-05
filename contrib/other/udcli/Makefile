SDK_DIR = ../../sdk
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0
CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L ./libudis86

default:
	kos32-gcc $(CFLAGS) $(INCLUDES) -o udcli.o udcli.c
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem console -o udcli udcli.o -ludis86 -lgcc -lc.dll
	objcopy udcli -O binary
	rm *.o
