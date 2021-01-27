KTCC_DIR=../../../develop/ktcc/trunk

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=json.c
CFLAGS=-c -nostdinc -I $(KTCC_DIR)/libc/include -I.

all:
	$(KTCC) $(CFLAGS) $(SRC) 
	ar -crs libjson.a *.o
clean:
	rm -f *.o *.a
