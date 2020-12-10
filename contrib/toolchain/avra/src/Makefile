CC = kos32-gcc
LD = kos32-ld 

SDK_DIR:= $(abspath ../../../sdk)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

CFLAGS = -c -fno-ident -Wall -O3 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

SOURCES = avra.c device.c parser.c expr.c mnemonic.c directiv.c \
		  macro.c file.c map.c coff.c args.c stdextra.c

OBJECTS = $(SOURCES:.c=.o)

OBJ_ALL = $(OBJECTS)

#********************************************************************

default: avra

avra: $(OBJ_ALL)
	$(LD) $(OBJ_ALL) $(LDFLAGS) $(LIBPATH) -o avra -lgcc -lc.dll
	objcopy avra -O binary

%.o : %.c Makefile	
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -f avra *.o *.p *~
