CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I ../fitz

DRAW_SRC := $(notdir $(wildcard *.c))
OBJECTS =  $(patsubst %.c, %.o, $(DRAW_SRC))

default: $(patsubst %.c,%.o,$(DRAW_SRC))
	ar rcs libdraw.a $(OBJECTS)
	rm *.o
	mv libdraw.a ../lib

%.o : %.c Makefile $(DRAW_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
