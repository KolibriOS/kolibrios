CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../sdk)
LDFLAGS = -static -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I ../SYSCALL/include -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/freetype/include -I $(SDK_DIR)/sources/zlib -I ../fitz -I ../pdf
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L ../lib

APPS_SRC := $(notdir $(wildcard *.c))

default: $(patsubst %.c,%.o,$(APPS_SRC))
	$(LD) $(LDFLAGS) $(LIBPATH) -o updf kolibri.o pdfapp.o kos_main.o -lmupdf -lfreetype -lfitz -lgcc -lfitz  -ldraw -ljpeg -ljbig2dec -lfreetype -lc.dll -lopenjpeg -lz -lc -lsyscall  --subsystem native
#	yet not work
#	$(LD) $(LDFLAGS) $(LIBPATH) -o pdfdraw kolibri.o pdfapp.o pdfdraw.o -lmupdf -lfreetype -lfitz -lgcc -lfitz  -ldraw -ljpeg -ljbig2dec -lfreetype -lc.dll -lopenjpeg -lz -lc -lsyscall  --subsystem native
	objcopy updf -O binary


%.o : %.c Makefile $(APPS_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
