KTCC_DIR=../../develop/ktcc/trunk
KLIBC_DIR = ../../develop/libraries/kolibri-libc

NAME=whois

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=whois.c
CFLAGS= -I$(KLIBC_DIR)/include
LFLAGS= -nostdlib -L $(KLIBC_DIR)/lib $(KLIBC_DIR)/lib/crt0.o
LIBS = -ltcc -lnetwork -lc.obj

all:
	$(KTCC) $(CFLAGS) $(LFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
