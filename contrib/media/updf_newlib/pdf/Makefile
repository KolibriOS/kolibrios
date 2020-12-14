CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../sdk)
LDFLAGS = -call_shared -nostdlib -T $(SDK_DIR)/sources/newlib/app-dynamic.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I ../fitz -I $(SDK_DIR)/sources/freetype/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

PDF_SRC := $(notdir $(wildcard *.c))
OBJECTS =  $(patsubst %.c, %.o, $(PDF_SRC))

default: $(patsubst %.c,%.o,$(PDF_SRC))
	ar rcs libmupdf.a *.o
	rm *.o
	mv libmupdf.a ../lib

%.o : %.c Makefile $(PDF_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
