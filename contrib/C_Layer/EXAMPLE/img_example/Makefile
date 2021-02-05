NAME=img_example
CC = kos32-gcc
LD = kos32-ld

SDK_DIR:= $(abspath ../../../sdk)
CLAYER:= $(abspath ../../)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds \
          --image-base 0 

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -std=c99

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include -I$(CLAYER)/INCLUDE
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

OBJPATH = $(CLAYER)/OBJ

all: 
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) main.c
	$(LD) $(LIBPATH) $(LDFLAGS) --subsystem native main.o $(OBJPATH)/loadlibimg.obj $(OBJPATH)/loadboxlib.obj -o $(NAME) -lgcc -lc.dll 
	strip -S $(NAME)
	objcopy $(NAME) -O binary

clean:
	-rm -f *.o $(NAME)
