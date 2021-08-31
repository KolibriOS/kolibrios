CC = kos32-gcc
LD = kos32-ld
OBJCOPY = kos32-objcopy
KPACK = kpack

TYRIAN = opentyrian

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident \
	-U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -D_KOLIBRI \
	-DTYRIAN_DIR='"/kolibrios/games/tyrian"' -DSDL_strlcpy=strncpy \
	-D_GNU_SOURCE=1 -D_REENTRANT -DNDEBUG -Wno-missing-field-initializers

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds -Map=output.map  --image-base 0  --subsystem native

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -Isrc
LIBPATH  = -L$(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L $(SDK_DIR)/lib

OBJS =  ./src/scroller.o         \
	./src/config.o           \
	./src/game_menu.o        \
	./src/file.o             \
	./src/opentyr.o          \
	./src/sndmast.o          \
	./src/sizebuf.o          \
	./src/video_scale.o      \
	./src/loudness.o         \
	./src/palette.o          \
	./src/joystick.o         \
	./src/lds_play.o         \
	./src/font.o             \
	./src/config_file.o      \
	./src/network.o          \
	./src/helptext.o         \
	./src/xmas.o             \
	./src/keyboard.o         \
	./src/jukebox.o          \
	./src/picload.o          \
	./src/shots.o            \
	./src/setup.o            \
	./src/mouse.o            \
	./src/musmast.o          \
	./src/nortvars.o         \
	./src/backgrnd.o         \
	./src/destruct.o         \
	./src/lvllib.o           \
	./src/video_scale_hqNx.o \
	./src/std_support.o      \
	./src/mtrand.o           \
	./src/sprite.o           \
	./src/episodes.o         \
	./src/arg_parse.o        \
	./src/opl.o              \
	./src/video.o            \
	./src/editship.o         \
	./src/vga_palette.o      \
	./src/pcxload.o          \
	./src/fonthand.o         \
	./src/mainint.o          \
	./src/tyrian2.o          \
	./src/lvlmast.o          \
	./src/animlib.o          \
	./src/pcxmast.o          \
	./src/menus.o            \
	./src/starlib.o          \
	./src/player.o           \
	./src/nortsong.o         \
	./src/vga256d.o          \
	./src/varz.o             \
	./src/params.o           \
	./SDL/joystick_stub.o    \
	./SDL/SDL_wave.o         \
	./SDL/SDL_audiocvt.o     \
	./SDL/uSDL.o

LIBS = -lgcc -lSDLn -lsound -lc.dll

$(TYRIAN): $(OBJS)
	$(LD) $(LDFLAGS) $(LIBPATH) $(OBJS) -o $(TYRIAN) $(LIBS)
	$(OBJCOPY) $(TYRIAN) -O binary
	$(KPACK) --nologo $(TYRIAN)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm src/*.o
