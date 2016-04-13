SDK_DIR:= $(abspath ../../../sdk)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

CFLAGS = -c -fno-ident -Wall -O3 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

CC = kos32-gcc $(CFLAGS) $(INCLUDES)
LD = kos32-ld 

SOURCES = avra.c device.c parser.c expr.c mnemonic.c directiv.c macro.c file.c map.c coff.c

OBJECTS = $(SOURCES:.c=.o)

OBJ_ALL = $(OBJECTS) args.o stdextra.o

#********************************************************************

default: avra

avra: $(OBJ_ALL)
	$(LD) $(OBJ_ALL) $(LDFLAGS) $(LIBPATH) -o avra -lgcc -lc.dll -lapp
	objcopy avra -O binary

args.o: args.c misc.h args.h 
avra.o: avra.c misc.h args.h avra.h device.h
device.o: device.c misc.h avra.h device.h
directiv.o: directiv.c misc.h args.h avra.h device.h
expr.o: expr.c misc.h avra.h
file.o: file.c misc.h avra.h
macro.o: macro.c misc.h args.h avra.h
mnemonic.o: mnemonic.c misc.h avra.h device.h
parser.o: parser.c misc.h avra.h
stdextra.o: stdextra.c misc.h
coff.o: coff.c coff.h

clean:
	rm -f avra *.o *.p *~
