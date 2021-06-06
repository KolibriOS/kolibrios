NEWLIB_INCLUDES=D:\KOSSDK\newlib\libc\include
APP_DYNAMIC_LDS=D:\KOSSDK\newlib/app-dynamic.lds
LIBDIR=D:\KOSSDK\kos32-msys-5.4.0\win32\lib
MAIN_TARGET=libkosnet_demo

CC=kos32-gcc
LD=kos32-ld
OBJCOPY=kos32-objcopy

CCFLAGS=-c -fomit-frame-pointer -I $(NEWLIB_INCLUDES) -I../include -Wall -Wextra
LDFLAGS=-call_shared -nostdlib --subsystem console -T $(APP_DYNAMIC_LDS) --image-base 0 -L $(LIBDIR) -L ../  -lkosnet -lgcc -lapp -lc.dll

all: libkosnet_demo

libkosnet_demo: libkosnet_demo.o
	$(LD) libkosnet_demo.o -o $(MAIN_TARGET) $(LDFLAGS)
	$(OBJCOPY) $(MAIN_TARGET) -O binary

libkosnet_demo.o: libkosnet_demo.c
	$(CC) $(CCFLAGS) libkosnet_demo.c -o libkosnet_demo.o

clean:
	del *.o
	del $(MAIN_TARGET)