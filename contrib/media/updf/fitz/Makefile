CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/freetype/include -I $(SDK_DIR)/sources/libjpeg -I $(SDK_DIR)/sources/zlib -I $(SDK_DIR)/sources/libopenjpeg -I $(SDK_DIR)/sources/libjbig2dec

FITZ_SRC := $(notdir $(wildcard *.c))
OBJECTS =  $(patsubst %.c, %.o, $(FITZ_SRC))

default: $(patsubst %.c,%.o,$(FITZ_SRC))
	ar rcs libfitz.a *.o
	rm *.o
	mv libfitz.a ../lib

%.o : %.c Makefile $(FITZ_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
