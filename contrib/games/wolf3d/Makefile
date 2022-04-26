CC = kos32-gcc
LD = kos32-ld
OBJCOPY = kos32-objcopy
KPACK = kpack

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -D_KOLIBRI
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0 --subsystem native

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I. -I SDL_mixer
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L $(SDK_DIR)/lib

TARGET = bin/wolf3d

OBJECTS += wl_cloudsky.o  
OBJECTS += wl_debug.o  
OBJECTS += id_sd.o  
OBJECTS += wl_play.o  
OBJECTS += id_vl.o  
OBJECTS += wl_act2.o  
OBJECTS += wl_floorceiling.o  
OBJECTS += wl_dir3dspr.o  
OBJECTS += wl_state.o  
OBJECTS += wl_atmos.o  
OBJECTS += id_in.o  
OBJECTS += signon.o  
OBJECTS += wl_parallax.o  
OBJECTS += wl_agent.o  
OBJECTS += sdl_winmain.o  
OBJECTS += wl_inter.o  
OBJECTS += wl_text.o  
OBJECTS += id_pm.o  
OBJECTS += wl_draw.o  
OBJECTS += wl_menu.o  
OBJECTS += wl_game.o  
OBJECTS += wl_act1.o  
OBJECTS += wl_main.o  
OBJECTS += wl_shade.o  
OBJECTS += id_us_1.o  
OBJECTS += id_vh.o  
OBJECTS += id_ca.o  
OBJECTS += joystick_stub.o 
OBJECTS += kolibri.o
OBJECTS += mame/fmopl.o


SDL_MIX_OBJ += SDL_mixer/mixer.o
SDL_MIX_OBJ += SDL_mixer/music.o
SDL_MIX_OBJ += SDL_mixer/load_aiff.o
SDL_MIX_OBJ += SDL_mixer/load_voc.o
SDL_MIX_OBJ += SDL_mixer/effects_internal.o
SDL_MIX_OBJ += SDL_mixer/effect_position.o

LIBS = -lSDLn -lsound -lgcc -lc.dll

$(TARGET): $(OBJECTS) $(SDL_MIX_OBJ) $(SDL_OBJ)
	$(LD) $(LDFLAGS) $(LIBPATH) -o $(TARGET) $(OBJECTS) $(SDL_MIX_OBJ) $(SDL_OBJ) $(LIBS)
	$(OBJCOPY) $(TARGET) -O binary
	$(KPACK) --nologo $(TARGET)

%.o : %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f $(OBJECTS)
