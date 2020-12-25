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

KOLIBRI_M_OBJ=  ../modules/kolibri/init.o  \
		../modules/kolibri/fs.o    \
		../modules/kolibri/net.o   \
		../modules/kolibri/gui.o   \
		../modules/kolibri/debug.o \
		
		
OBJECTS =  tpmain.o $(KOLIBRI_M_OBJ)

all:$(NAME)

$(NAME): $(OBJECTS)
	$(LD) $(LIBPATH) --subsystem native -o $@ $(OBJECTS) $(LDFLAGS) -n -Map $(NAME).map
	kos32-objcopy $@ -O binary
	kpack $@

%.o : %.c Makefile	
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o $(NAME).map $(KOLIBRI_M_OBJ)
