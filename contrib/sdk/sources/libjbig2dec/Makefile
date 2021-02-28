CC = kos32-gcc
LD = kos32-ld

SDK_DIR = /media/maxim/E22406D62406AE1B/SVNKOS/contrib/sdk
CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/freetype/include -I $(SDK_DIR)/sources/libpng -I $(SDK_DIR)/sources/zlib -I .

SRC := $(notdir $(wildcard *.c))
OBJECTS =  $(patsubst %.c, %.o, $(SRC))

default: $(patsubst %.c,%.o,$(SRC))
	ar rcs libjbig2dec.a *.o
	rm *.o
	mv libjbig2dec.a ../../lib

%.o : %.c Makefile $(SRC)
	$(CC) $(CFLAGS) -DHAVE_CONFIG_H $(INCLUDES) -o $@ $<
