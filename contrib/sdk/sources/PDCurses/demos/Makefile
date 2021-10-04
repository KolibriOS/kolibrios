CC = kos32-gcc
LD = kos32-ld
OBJCOPY = kos32-objcopy
KPACK = kpack

BIN = xmas

SDK_DIR = $(abspath ../../../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident \
	-U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -D_KOLIBRI \
	-DSDL_strlcpy=strncpy -D_GNU_SOURCE=1 -D_REENTRANT -DNDEBUG -Wno-missing-field-initializers \

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds -Map=output.map  --image-base 0  --subsystem native

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I..
LIBPATH  = -L$(SDK_DIR)/lib

OBJS = xmas.o 

LIBS = -lgcc -lcurses -lSDLn -lsound -lc.dll

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $(LIBPATH) $(OBJS) -o $(BIN) $(LIBS)
	$(OBJCOPY) $(BIN) -O binary
	$(KPACK) --nologo $(BIN)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm src/*.o
