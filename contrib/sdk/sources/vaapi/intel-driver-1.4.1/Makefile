LIBRARY = i965-video

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -Wall -Winline -fno-ident
CFLAGS+= -mno-ms-bitfields -fomit-frame-pointer

LDFLAGS = -shared -s -nostdlib -T ../../newlib/dll.lds --entry _DllStartup --image-base=0
LDFLAGS+= --version-script i965-video.ver --out-implib lib$(LIBRARY).dll.a

ARFLAGS = crs

INCLUDES= -I../../newlib/libc/include -I../../libdrm -I../../libdrm/intel -I../../libdrm/include/drm
INCLUDES+= -I../libva-1.4.1 

LIBPATH:= -L../../../lib -L/home/autobuild/tools/win32/mingw32/lib

LIBS:=  -ldll -ldrm -lc.dll -lgcc

DEFINES:= -DHAVE_CONFIG_H

SOURCES =				\
	src/gen6_mfc.c			\
	src/gen6_mfc_common.c		\
	src/gen6_mfd.c			\
	src/gen6_vme.c			\
	src/gen7_vme.c			\
	src/gen7_mfc.c			\
	src/gen7_mfd.c			\
	src/gen75_mfd.c			\
	src/gen75_mfc.c			\
	src/gen8_mfc.c			\
	src/gen8_mfd.c			\
	src/gen8_vme.c			\
	src/gen75_picture_process.c	\
	src/gen75_vme.c			\
	src/gen75_vpp_gpe.c		\
	src/gen75_vpp_vebox.c		\
	src/i965_avc_bsd.c		\
	src/i965_avc_hw_scoreboard.c	\
	src/i965_avc_ildb.c		\
	src/i965_decoder_utils.c	\
	src/i965_device_info.c		\
	src/i965_drv_video.c		\
	src/i965_encoder.c		\
	src/i965_encoder_utils.c	\
	src/i965_media.c		\
	src/i965_media_h264.c		\
	src/i965_media_mpeg2.c		\
	src/i965_gpe_utils.c		\
	src/i965_post_processing.c	\
	src/gen8_post_processing.c	\
	src/i965_render.c		\
	src/gen8_render.c		\
	src/intel_batchbuffer.c		\
	src/intel_driver.c		\
	src/intel_memman.c		\
	src/object_heap.c		\
	src/intel_media_common.c

 
OBJECTS     = $(patsubst %.c, %.o, $(SOURCES))

# targets 

all: $(LIBRARY).dll

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)
	$(STRIP) $@
	mv -f $@ ../../../bin
	mv -f lib$(LIBRARY).dll.a ../../../lib  
	

%.o : %.c Makefile
	$(CC) $(INCLUDES) $(CFLAGS) $(DEFINES)  -o $@ $<


clean: 
	-rm -f *.o 
 