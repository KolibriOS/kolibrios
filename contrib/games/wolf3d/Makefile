CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I.
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

OBJECTS = wl_cloudsky.o  wl_debug.o  id_sd.o  wl_play.o  id_vl.o  wl_act2.o  wl_floorceiling.o  wl_dir3dspr.o  wl_state.o  wl_atmos.o  id_in.o  signon.o  wl_parallax.o  wl_agent.o  sdl_winmain.o  wl_inter.o  wl_text.o  id_pm.o  wl_draw.o  wl_menu.o  wl_game.o  wl_act1.o  wl_main.o  wl_shade.o  id_us_1.o  id_vh.o  id_ca.o  joystick_stub.o kolibri.o

default: $(OBJECTS)
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem native -o wolf3d $(OBJECTS) -lSDLn -lsound -lstdc++ -lsupc++ -lgcc -lc.dll
	objcopy wolf3d -O binary
	kpack --nologo wolf3d

%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
