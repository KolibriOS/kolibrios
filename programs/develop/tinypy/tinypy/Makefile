NAME=tinypy
CC = kos32-gcc
LD = kos32-ld

SDK_DIR:=$(abspath ../../../../contrib/sdk)

LDFLAGS = -static -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds \
          --image-base 0 -lgcc -lSDLn  -lc.dll -lsound

SDL_DIR = $(SDK_DIR)/sources/SDL-1.2.2_newlib
          
CFLAGS = -DCONIO -U_Win32 -U_WIN32 -U__MINGW32__ -mpreferred-stack-boundary=2 \
			-mincoming-stack-boundary=2 -fno-builtin -fno-common  -O0 

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDL_DIR)/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L.  

STD_MODULES= ../std_modules/modules.o
		
OBJECTS =  $(STD_MODULES) tpmain.o

all:$(NAME)

$(NAME): $(OBJECTS)
	$(LD) $(LIBPATH) --subsystem native -o $@ $(OBJECTS) $(LDFLAGS) -n -Map $(NAME).map
	strip -S $(NAME)
	kos32-objcopy $@ -O binary
	kpack $@

%.o : %.c Makefile	
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -f $(NAME).map $(OBJECTS) *.o core/*.tpc core/*.pyc
