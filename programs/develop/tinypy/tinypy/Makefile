NAME = tinypy
CC = kos32-gcc
LD = kos32-ld

SDK_DIR:= $(abspath ../../../../contrib/sdk)

LDFLAGS = -static -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds \
          --image-base 0 -lgcc -lc.dll

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -mpreferred-stack-boundary=2 \
			-mincoming-stack-boundary=2 -fno-builtin -fno-common

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

MODULES_DIR = $(abspath ../modules)

MODULES_OBJ =	$(MODULES_DIR)/kolibri/init.o  \
				$(MODULES_DIR)/kolibri/fs.o    \
				$(MODULES_DIR)/kolibri/net.o   \
				$(MODULES_DIR)/kolibri/gui.o   \
				$(MODULES_DIR)/kolibri/debug.o

OBJECTS = tpmain.o $(MODULES_OBJ)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(LD) $(LIBPATH) --subsystem native -o $@ $(OBJECTS) $(LDFLAGS) -n # -Map $(NAME).map
	objcopy $@ -O binary

%.o : %.c Makefile	
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o $(NAME).map $(MODULES_OBJ)
