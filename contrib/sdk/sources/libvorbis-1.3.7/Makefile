CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

LIBNAME=libvorbis

SDK_DIR:= $(abspath ../../)

OBJS =  lib/block.o \
	lib/sharedbook.o \
	lib/vorbisenc.o \
	lib/info.o \
	lib/registry.o \
	lib/psy.o \
	lib/window.o \
	lib/lpc.o \
	lib/tone.o \
	lib/smallft.o \
	lib/barkmel.o \
	lib/mdct.o \
	lib/bitrate.o \
	lib/analysis.o \
	lib/vorbisfile.o \
	lib/res0.o \
	lib/lookup.o \
	lib/lsp.o \
	lib/floor1.o \
	lib/floor0.o \
	lib/codebook.o \
	lib/envelope.o \
	lib/mapping0.o \
	lib/synthesis.o


CFLAGS = -c -O2 -fno-pie -mpreferred-stack-boundary=2 -fno-ident -fomit-frame-pointer -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -fno-exceptions -fno-asynchronous-unwind-tables -ffast-math -mno-ms-bitfields -march=pentium-mmx -UWIN32 -U_Win32 -U_WIN32 -U__MINGW32__ -DHAVE_CONFIG

CFLAGS += -I../newlib/libc/include -Iinclude  -I../libogg-1.3.5/include -Ilib

all: $(LIBNAME).a 

$(LIBNAME).a: $(OBJS)
	$(AR) -crs $(SDK_DIR)/lib/$(LIBNAME).a $(OBJS)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f */*.o \ rm *.o \ rm */*/*.o
