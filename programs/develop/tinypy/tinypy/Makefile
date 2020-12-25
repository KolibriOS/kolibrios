NAME=tinypy
CC = kos32-gcc
LD = kos32-ld

SDK_DIR:= $(abspath ../../../../contrib/sdk)

LDFLAGS = -static -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds \
          --image-base 0 -lgcc -lc.dll

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -mpreferred-stack-boundary=2 \
			-mincoming-stack-boundary=2 -fno-builtin -fno-common

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib


OBJECTS =  tpmain.o kolibri_init.o kolibri_fs.o kolibri_gui.o kolibri_net.o kolibri_dbg.obj

all:$(NAME)

$(NAME): $(OBJECTS) Makefile
	$(LD) $(LIBPATH) --subsystem native -o $@ $(OBJECTS) $(LDFLAGS) -n -Map $(NAME).map
	kos32-objcopy $@ -O binary

%.o : %.c Makefile	
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

kolibri_dbg.obj: fasm_modules/kolibri_dbg.s
	fasm fasm_modules/kolibri_dbg.s
	cp fasm_modules/kolibri_dbg.obj .

clean:
	-rm -f *.o $(NAME).map fasm_modules/kolibri_dbg.obj kolibri_dbg.obj
