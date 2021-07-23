FASM = fasm
TCC_DIR = /home/max/kolibri-svn/programs/develop/ktcc/trunk
TCC= $(TCC_DIR)/bin/kos32-tcc

CFLAGS= -I../../include -I$(TCC_DIR)/libc.obj/include -stack=10000

all:
	$(FASM) load_mbedtls.asm
	$(TCC) $(CFLAGS) ssl_client1.c load_mbedtls.o -o ssl_client1 -ltcc -lc.obj

clean:
	rm -f *.o ssl_client1 
