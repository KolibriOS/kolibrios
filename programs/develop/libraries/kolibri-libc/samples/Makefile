
KTCC=../../../ktcc/trunk/bin/kos32-tcc
FASM= fasm
KPACK = kpack
CFLAGS = -I../include 
LDFLAGS = -nostdlib -L../lib ../lib/crt0.o

BIN= stdio_test.kex \
basic_gui.kex \
http_tcp_demo.kex \
math_test.kex \
string_test.kex \
whois.kex  \
file_io.kex \
tmpdisk_work.kex \
fasm/sprintf_test.kex

all: $(BIN)
	
%.kex : %.c
	$(KTCC) $(CFLAGS) $(LDFLAGS) $< -o $@ -ltcc -lnetwork -lc.obj
	$(KPACK) --nologo $@
	
%.kex : %.asm
	$(FASM) $< $@
	$(KPACK) --nologo $@

clean:
	rm *.kex
