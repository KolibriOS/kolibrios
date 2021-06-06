NEWLIB_INCLUDES=D:\KOSSDK\newlib\libc\include
APP_DYNAMIC_LDS=D:\KOSSDK\newlib/app-dynamic.lds
LIBDIR=D:\KOSSDK\kos32-msys-5.4.0\win32\lib
MAIN_TARGET=ssl_client1

CC=kos32-gcc
LD=kos32-ld
OBJCOPY=kos32-objcopy

CCFLAGS=-c -fomit-frame-pointer -I $(NEWLIB_INCLUDES) -I../../include -I../../kosnet/include -Wall -Wextra
LDFLAGS=-call_shared -nostdlib --subsystem console -T $(APP_DYNAMIC_LDS) --image-base 0 -L $(LIBDIR) -L ../../kosnet -L ../../library -lmbedtls -lmbedx509 -lmbedcrypto -lkosnet -lgcc -lapp -lc.dll

all: ssl_client1

ssl_client1: ssl_client1.o
	$(LD) ssl_client1.o -o $(MAIN_TARGET) $(LDFLAGS)
	$(OBJCOPY) $(MAIN_TARGET) -O binary

ssl_client1.o: ssl_client1.c
	$(CC) $(CCFLAGS) ssl_client1.c -o ssl_client1.o

clean:
	del *.o
	del $(MAIN_TARGET)