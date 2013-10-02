
LIBRARY = libmpg123

CC = gcc
CFLAGS= -c -O2 -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math 
CPPFLAGS =  -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_I386 -DOPT_I586 -DOPT_I586_DITHER -DOPT_MMX -DOPT_3DNOW -DOPT_3DNOWEXT -DOPT_SSE -DREAL_IS_FLOAT -DNOXFERMEM -DNEWOLD_WRITE_SAMPLE

LD = ld
LDFLAGS = -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --image-base=0 --out-implib $(LIBRARY).dll.a

STRIP = $(PREFIX)strip

UDEF= -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER 
DEFINES= -DHAVE_CONFIG_H

INCLUDES= -I../newlib/include

LIBPATH:= -L../newlib

LIBS:=  -ldll -lc.dll -lgcc


SRCS=				  		\
			compat.c		\
			dct64.c			\
			dct64_i386.c		\
			dither.c		\
			equalizer.c		\
			feature.c		\
			format.c		\
			frame.c			\
			icy.c			\
			icy2utf8.c		\
			id3.c			\
			index.c			\
			layer1.c		\
			layer2.c		\
			layer3.c		\
			lfs_alias.c		\
			libmpg123.c		\
			ntom.c			\
			optimize.c		\
			parse.c			\
			readers.c		\
			stringbuf.c		\
			synth.c			\
			tabinit.c

ASM=						\
			dct36_3dnow.S		\
			dct36_3dnowext.S	\
			dct64_3dnow.S		\
			dct64_3dnowext.S	\
			dct64_mmx.S		\
			dct64_sse.S		\
			dct64_sse_float.S	\
			equalizer_3dnow.S	\
			getcpuflags.S		\
			synth_3dnow.S		\
			synth_3dnowext.S	\
			synth_i586.S		\
			synth_i586_dither.S	\
			synth_mmx.S		\
			synth_sse.S		\
			synth_sse_float.S	\
			synth_sse_s32.S		\
			synth_stereo_sse_float.S\
			synth_stereo_sse_S32.S	\
			tabinit_mmx.S

ASM_OBJS =  $(patsubst %.S, %.o, $(ASM))
SRCS_OBJ = $(patsubst %.c, %.o, $(SRCS))


all: $(LIBRARY).dll

$(LIBRARY).dll: $(SRCS_OBJ) $(ASM_OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ libmpg123.dll.def $(SRCS_OBJ) $(ASM_OBJS) $(LIBS)
	$(STRIP) $@
	sed -f ../newlib/cmd1.sed libmpg123.dll.def > mem
	sed -f ../newlib/cmd2.sed mem >$(LIBRARY).inc

%.o : %.S $(ASM) Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UDEF) $(DEFINES) $(INCLUDES) -o $@ $<

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UDEF) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 

 