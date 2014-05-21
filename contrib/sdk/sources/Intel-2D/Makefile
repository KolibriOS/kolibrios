LIBRARY= pixlib

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

LDFLAGS:=  -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --image-base=0 
PXFLAGS:= --version-script pixlib.ver --out-implib $(LIBRARY).dll.a
SNAFLAGS:= --version-script sna.ver --output-def sna.def
UXAFLAGS:= --version-script uxa.ver --output-def uxa.def
 
INCLUDES= -I. -I../libdrm/intel -I../libdrm/include/drm -I./render_program -I../pixman -I../newlib/libc/include

LIBPATH:= -L../../lib

LIBS:=  -ldll -lc.dll

DEFINES:= -DHAS_DEBUG_FULL=0 -DSHOW_BATCH=0 -DDEBUG_DUMP=0

SRC_PIXLIB = 	pixlib.c

SRC_SNA = 				\
		sna/gen4_common.c	\
		sna/gen6_common.c	\
		sna/gen3_render.c	\
		sna/gen4_render.c	\
		sna/gen4_vertex.c	\
		sna/gen5_render.c	\
		sna/gen6_render.c	\
		sna/gen7_render.c	\
		sna/kgem.c		\
		sna/sna.c		\
		sna/sna_cpu.c		\
		sna/sna_stream.c	\
		sna/sna_transform.c	\
		sna/utils.c		\
		sna/brw/brw_eu.c	\
		sna/brw/brw_eu_emit.c	\
		sna/brw/brw_sf.c	\
		sna/brw/brw_wm.c

SRC_UXA = 	pixlib_uxa.c		\
		uxa/uxa.c		\
		uxa/i965_3d.c		\
		uxa/i965_render.c	\
		uxa/intel_batchbuffer.c

OBJ_PIXLIB =  $(patsubst %.c, %.o, $(SRC_PIXLIB))
OBJ_UXA =   $(patsubst %.c, %.o, $(SRC_UXA))
OBJ_SNA =   $(patsubst %.c, %.o, $(SRC_SNA))
 
ifeq ($(findstring ebox,$(MAKECMDGOALS)),ebox)
CFLAGS+=-march=pentium-mmx
DEFINES+= -DBUILD_EBOX
else
LIBS+= -ldrm.dll
ifeq ($(findstring uxa,$(MAKECMDGOALS)),uxa)
OBJECTS= $(OBJ_UXA)
DEFINES+= -DBUILD_UXA
else
OBJECTS= $(OBJ_SNA)
DEFINES+= -DBUILD_SNA
LIBS+= -lgcc
LIBPATH+= -L/home/autobuild/tools/win32/mingw32/lib
endif
endif

# targets 

all:$(LIBRARY).dll intel-sna.drv
uxa:$(LIBRARY).dll intel-uxa.drv
ebox:$(LIBRARY).dll


$(LIBRARY).dll: $(OBJ_PIXLIB) Makefile
	$(LD) $(LDFLAGS) $(PXFLAGS) $(LIBPATH) -o $@ $(OBJ_PIXLIB) $(LIBS)
	mv -f $@ ../../bin
	mv -f $(LIBRARY).dll.a ../../lib   

intel-sna.drv: $(OBJ_SNA) Makefile
	$(LD) $(LDFLAGS) $(SNAFLAGS) $(LIBPATH) -o $@ $(OBJ_SNA) $(LIBS)
	mv -f $@ ../../bin

intel-uxa.drv: $(OBJ_UXA) Makefile
	$(LD) $(LDFLAGS) $(UXAFLAGS) $(LIBPATH) -o $@ $(OBJ_UXA) $(LIBS)
	$(STRIP) $@
	mv -f $@ ../../bin
 
%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 




