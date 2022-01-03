CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

LIBNAME=libSDL_mixer

SDK_DIR:= $(abspath ../../../sdk)

OBJS =  effect_stereoreverse.o \
	effect_position.o \
	effects_internal.o \
	music.o \
	mixer.o \
	load_ogg.o \
	music_ogg.o \
	dynamic_ogg.o \
	wavestream.o \
	load_aiff.o \
	load_voc.o
 
CFLAGS = -c -O2 -mpreferred-stack-boundary=2 -fno-ident -fomit-frame-pointer -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -fno-exceptions -fno-asynchronous-unwind-tables -ffast-math -mno-ms-bitfields -march=pentium-mmx -UWIN32 -U_Win32 -U_WIN32 -U__MINGW32__ -I../newlib/libc/include/ -I../SDL-1.2.2_newlib/include -I../libogg-1.3.5/include -I.. -I../libvorbis-1.3.7/include -DOGG_MUSIC

all: $(LIBNAME).a 

$(LIBNAME).a: $(OBJS)
	$(AR) -crs $(SDK_DIR)/lib/$(LIBNAME).a $(OBJS)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f */*.o \ rm *.o \ rm */*/*.o
