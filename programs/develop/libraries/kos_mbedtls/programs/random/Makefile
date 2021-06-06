NEWLIB_INCLUDES=D:\KOSSDK\newlib\libc\include
APP_DYNAMIC_LDS=D:\KOSSDK\newlib/app-dynamic.lds
LIBDIR=D:\KOSSDK\kos32-msys-5.4.0\win32\lib
TARGET_1=gen_entropy
TARGET_2=gen_random_ctr_drbg

CC=kos32-gcc
LD=kos32-ld
OBJCOPY=kos32-objcopy

CCFLAGS=-c -fomit-frame-pointer -I $(NEWLIB_INCLUDES) -I../../include -Wall -Wextra
LDFLAGS=-call_shared -nostdlib --subsystem console -T $(APP_DYNAMIC_LDS) --image-base 0 -L $(LIBDIR) -L ../../library -lmbedtls -lmbedx509 -lmbedcrypto -lgcc -lapp -lc.dll

all: gen_entropy gen_random_ctr_drbg

gen_entropy: gen_entropy.o
	$(LD) gen_entropy.o -o $(TARGET_1) $(LDFLAGS)
	$(OBJCOPY) $(TARGET_1) -O binary

gen_entropy.o: gen_entropy.c
	$(CC) $(CCFLAGS) gen_entropy.c -o gen_entropy.o

gen_random_ctr_drbg: gen_random_ctr_drbg.o
	$(LD) gen_random_ctr_drbg.o -o $(TARGET_2) $(LDFLAGS)
	$(OBJCOPY) $(TARGET_2) -O binary

gen_random_ctr_drbg.o: gen_random_ctr_drbg.c
	$(CC) $(CCFLAGS) gen_random_ctr_drbg.c -o gen_random_ctr_drbg.o

clean:
	del *.o
	del $(TARGET_1)
	del $(TARGET_2)