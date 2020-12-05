SDK_DIR = ../../../sdk
CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include

default:
	kos32-gcc $(CFLAGS) $(INCLUDES) -o decode.o decode.c
	kos32-gcc $(CFLAGS) $(INCLUDES) -o input.o input.c
	kos32-gcc $(CFLAGS) $(INCLUDES) -o itab.o itab.c
	kos32-gcc $(CFLAGS) $(INCLUDES) -o syn-att.o syn-att.c
	kos32-gcc $(CFLAGS) $(INCLUDES) -o syn-intel.o syn-intel.c
	kos32-gcc $(CFLAGS) $(INCLUDES) -o syn.o syn.c
	kos32-gcc $(CFLAGS) $(INCLUDES) -o udis86.o udis86.c

	ar rcs libudis86.a decode.o input.o itab.o syn-att.o syn-intel.o syn.o udis86.o
	rm *.o
