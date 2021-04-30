
KTCC=../../../ktcc/trunk/bin/kos32-tcc
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
exp_drv_work.kex 


all: $(BIN)
	
%.kex : %.c
	$(KTCC) $(CFLAGS) $(LDFLAGS) $< -o $@ -lnetwork -lc.obj
	$(KPACK) --nologo $@

clean:
	rm *.kex
