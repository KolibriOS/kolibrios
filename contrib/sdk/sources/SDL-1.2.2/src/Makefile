CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld

SDK_DIR:= $(abspath ../../..)

endian_OBJS = endian/SDL_endian.o
file_OBJS = file/SDL_rwops.o
hermes_OBJS = hermes/mmxp2_32.obj hermes/mmx_main.obj hermes/x86p_16.obj \
	hermes/x86p_32.obj hermes/x86_main.obj
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
audio_OBJS = audio/SDL_kolibri_audio.o 
#\
#	audio/klbr_sdk/src/init.o audio/klbr_sdk/src/setbuf.o \
#	audio/klbr_sdk/src/stopbuf.o audio/klbr_sdk/src/sndgetsize.o

curr_OBJS = SDL.o SDL_error.o SDL_fatal.o SDL_getenv.o

OBJS = $(endian_OBJS) $(file_OBJS) $(hermes_OBJS) $(thread_OBJS) \
	$(timer_OBJS) $(event_OBJS) $(video_OBJS) $(curr_OBJS) $(audio_OBJS)

CFLAGS = -c -O2 -D_REENTRANT -I../include -I. -DPACKAGE=\"SDL\" -DVERSION=\"1.2.2\" \
	-fexpensive-optimizations -Wall -DENABLE_AUDIO -UDISABLE_AUDIO -DDISABLE_JOYSTICK \
	-DDISABLE_CDROM -DDISABLE_THREADS -DENABLE_TIMERS \
	-DUSE_ASMBLIT -Ihermes -Iaudio -Ivideo -Ievents \
	-Ijoystick -Icdrom -Ithread -Itimer -Iendian -Ifile -DENABLE_MENUETOS \
	-DNO_SIGNAL_H -DDISABLE_STDIO -DNEED_SDL_GETENV -DENABLE_FILE -UDISABLE_FILE \
	-D__MENUETOS__ -DDEBUG_VIDEO -UWIN32 -U_Win32 -U_WIN32 -U__MINGW32__ \
	-I../../newlib/libc/include/


LDFLAGS = -shared -s -T../../newlib/libc/app.lds -nostdlib --image-base 0 -L/home/autobuild/tools/win32/mingw32/lib -lgcc /home/autobuild/tools/win32/lib/libz.a /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libc.dll.a

ARFLAGS = crs

#include $(MENUETDEV)/makefiles/Makefile_for_lib

all: libSDL.a

install: libSDL.a
	mv -f libSDL.a $(SDK_DIR)/lib   

libSDL.a: $(OBJS)
	$(AR) $(ARFLAGS) libSDL.a

%.obj : %.asm Makefile
	nasm -f coff $< $

%.o : %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm -f */*.o


