LIBRARY = SDL

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

SDK_DIR:= $(abspath ../../..)

endian_OBJS = endian/SDL_endian.o
file_OBJS = file/SDL_rwops.o
hermes_OBJS = hermes/mmxp2_32.o hermes/mmx_main.o hermes/x86p_16.o \
	hermes/x86p_32.o hermes/x86_main.o
thread_OBJS = thread/SDL_syscond.o thread/SDL_sysmutex.o thread/SDL_syssem.o \
	thread/SDL_systhread.o thread/SDL_thread.o
timer_OBJS = timer/SDL_timer.o timer/dummy/SDL_systimer.o
event_OBJS = events/SDL_active.o events/SDL_events.o events/SDL_expose.o \
	events/SDL_keyboard.o events/SDL_mouse.o events/SDL_quit.o \
	events/SDL_resize.o
video_OBJS = video/SDL_blit_0.o video/SDL_blit_1.o video/SDL_blit_A.o \
	video/SDL_blit.o video/SDL_blit_N.o video/SDL_bmp.o \
	video/SDL_cursor.o video/SDL_gamma.o video/SDL_pixels.o \
	video/SDL_RLEaccel.o video/SDL_stretch.o video/SDL_surface.o \
	video/SDL_video.o video/SDL_yuv.o video/SDL_yuv_mmx.o \
	video/SDL_yuv_sw.o video/menuetos/SDL_menuetevents.o \
	video/menuetos/SDL_menuetvideo.o
audio_OBJS = ../../sound/src/init.obj ../../sound/src/setbuf.obj \
	../../sound/src/stopbuf.obj ../../sound/src/sndgetsize.obj  audio/SDL_kolibri_audio.o

curr_OBJS = SDL.o SDL_error.o SDL_fatal.o SDL_getenv.o

OBJECTS = $(endian_OBJS) $(file_OBJS) $(hermes_OBJS) $(thread_OBJS) \
	$(timer_OBJS) $(event_OBJS) $(video_OBJS) $(curr_OBJS) $(audio_OBJS)

CFLAGS = -c -O2 -D_REENTRANT -I../include -I. -DPACKAGE=\"SDL\" -DVERSION=\"1.2.2\" \
	-fexpensive-optimizations -Wall -DENABLE_AUDIO -UDISABLE_AUDIO -DDISABLE_JOYSTICK \
	-DDISABLE_CDROM -DDISABLE_THREADS -DENABLE_TIMERS \
	-DUSE_ASMBLIT -Ihermes -Iaudio -Ivideo -Ievents \
	-Ijoystick -Icdrom -Ithread -Itimer -Iendian -Ifile -DENABLE_MENUETOS \
	-DNO_SIGNAL_H -DDISABLE_STDIO -DNEED_SDL_GETENV -DENABLE_FILE -UDISABLE_FILE \
	-D__MENUETOS__ -DDEBUG_VIDEO -UWIN32 -U_Win32 -U_WIN32 -U__MINGW32__


LDFLAGS = -shared -s -nostdlib -T../../newlib/dll.lds --entry _DllStartup --image-base=0
# --image-base 0 -L/home/autobuild/tools/win32/mingw32/lib -lgcc /home/autobuild/tools/win32/lib/libz.a /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libc.dll.a

LDFLAGS+= --out-implib lib$(LIBRARY).dll.a

ARFLAGS = crs

INCLUDES= -I../../newlib/libc/include -I./include -I./include/freetype/config 

LIBPATH:= -L../../../lib -L/home/autobuild/tools/win32/mingw32/lib

LIBS:=  -ldll -lc.dll -lgcc

#include $(MENUETDEV)/makefiles/Makefile_for_lib

# all: libSDL.a

# install: libSDL.a
# 	mv -f libSDL.a $(SDK_DIR)/lib   

# libSDL.a: $(OBJS)
# 	$(AR) $(ARFLAGS) libSDL.a

# ../../sound/src/init.o : ../../sound/src/init.asm Makefile
# 	fasm ../../sound/src/init.asm ../../sound/src/init.o

# ../../sound/src/setbuf.o : ../../sound/src/setbuf.asm Makefile
# 	fasm ../../sound/src/setbuf.asm ../../sound/src/setbuf.o

# ../../sound/src/stopbuf.o :  ../../sound/src/stopbuf.asm
# 	fasm ../../sound/src/stopbuf.asm ../../sound/src/stopbuf.o

# ../../sound/src/sndgetsize.o : ../../sound/src/sndgetsize.asm
# 	fasm ../../sound/src/sndgetsize.asm ../../sound/src/sndgetsize.o

%.o : %.asm Makefile
	nasm -f coff $< -o $@

%.obj : %.asm Makefile
	fasm $< $@

# %.o : %.c Makefile
# 	$(CC) $(CFLAGS) -o $@ $<

# clean:
# 	-rm -f */*.o


all:lib$(LIBRARY).a $(LIBRARY).dll

lib$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) cvrs lib$(LIBRARY).a $(OBJECTS)
	mv -f lib$(LIBRARY).a ../../../lib

$(LIBRARY).dll: $(OBJECTS) Makefile
	$(warning "$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)")
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)
	$(STRIP) $@
	#sed -f ../newlib/cmd1.sed lib$(LIBRARY).def > mem
	#sed -f ../newlib/cmd2.sed mem >$(LIBRARY).inc
	mv -f $@ ../../../bin
	mv -f lib$(LIBRARY).dll.a ../../../lib  
	

%.o : %.c Makefile
	$(CC) $(INCLUDES) $(CFLAGS) $(DEFINES)  -o $@ $<

clean: 
	-rm -f */*/*.o 
