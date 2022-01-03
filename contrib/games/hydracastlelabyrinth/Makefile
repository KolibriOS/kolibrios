CC = kos32-gcc
LD = kos32-ld
OBJCOPY = kos32-objcopy
KPACK = kpack
STRIP = kos32-strip

HCL = hcl

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -O2 -std=c11 -mpreferred-stack-boundary=2 -fno-ident -fomit-frame-pointer -fno-stack-check \
	-fno-stack-protector -mno-stack-arg-probe -fno-exceptions -fno-asynchronous-unwind-tables \
	-ffast-math -mno-ms-bitfields -march=pentium-mmx \
	-U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -D_KOLIBRI \
	-D_GNU_SOURCE=1 -Wno-missing-field-initializers -D_SDL -DUSE_SDL=1 -DOGG_MUSIC

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds -Map=output.map  --image-base 0  --subsystem native

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I$(SDK_DIR)/sources/SDL_mixer-1.2.12 -Isrc 
LIBPATH  = -L$(SDK_DIR)/lib

GAME_OBJS = \
	src/collision.o \
	src/effect.o \
	src/enemy.o \
	src/game.o \
	src/hero.o \
	src/ini.o \
	src/inventory.o \
	src/main.o \
	src/object.o \
	src/options.o \
	src/PHL.o \
	src/platform.o \
	src/qda.o \
	src/stagedata.o \
	src/text.o \
	src/titlescreen.o \
	src/weapon.o \
	src/enemies/batboss.o \
	src/enemies/bat.o \
	src/enemies/bee.o \
	src/enemies/boar.o \
	src/enemies/boomknight.o \
	src/enemies/crab.o \
	src/enemies/devil.o \
	src/enemies/dodo.o \
	src/enemies/dog.o \
	src/enemies/firewheel.o \
	src/enemies/fish.o \
	src/enemies/garm.o \
	src/enemies/gas.o \
	src/enemies/ghoul.o \
	src/enemies/golem.o \
	src/enemies/gyra.o \
	src/enemies/heads.o \
	src/enemies/hydra.o \
	src/enemies/jellyfish.o \
	src/enemies/knight.o \
	src/enemies/lolidra.o \
	src/enemies/pendulum.o \
	src/enemies/podoboo.o \
	src/enemies/poisonknight.o \
	src/enemies/pumpkin.o \
	src/enemies/seal.o \
	src/enemies/skeleton.o \
	src/enemies/skull.o \
	src/enemies/slime.o \
	src/enemies/slug.o \
	src/enemies/thwomp.o \
	src/enemies/waterjumper.o \
	src/enemies/wizard.o

SDL_OBJS = src/sdl/audio.o \
	src/sdl/input.o \
	src/sdl/graphics.o \
	src/sdl/system.o \
	src/sdl/joystick_stub.o

MISC_OBJS = src/misc.o

LIBS = -lSDL_mixer -lvorbis -logg  -lSDLn -lsound  -lgcc -lc.dll 

$(HCL): $(GAME_OBJS) $(SDL_OBJS) $(MISC_OBJS)
	$(LD) $(LDFLAGS) $(LIBPATH) $(GAME_OBJS) $(SDL_OBJS) $(MISC_OBJS) -o $(HCL) $(LIBS)
	$(STRIP) -S $(HCL)
	$(OBJCOPY) $(HCL) -O binary
	$(KPACK) --nologo $(HCL)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm $(GAME_OBJS) $(SDL_OBJS)
