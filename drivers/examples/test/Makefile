
KTCC=kos32-tcc
KPACK = kpack
KLIBC =  ../../../programs/develop/ktcc/trunk/libc.obj
CFLAGS = -I $(KLIBC)/include 
LDFLAGS = -nostdlib -L$(KLIBC)/lib $(KLIBC)/lib/crt0.o

all:
	$(KTCC) $(CFLAGS) $(LDFLAGS) drv_test.c -o drv_test -lc.obj
	$(KPACK) --nologo drv_test
clean:
	rm *.kex
