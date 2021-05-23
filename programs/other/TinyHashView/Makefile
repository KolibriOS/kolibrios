KTCC_DIR=../../develop/ktcc/trunk
KLIBC_DIR = ../../develop/libraries/kolibri-libc

NAME=thashview

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=thashview.c

CFLAGS= -I$(KLIBC_DIR)/include -I$(KTCC_DIR)/libc/include
LFLAGS= -nobss -nostdlib -L $(KLIBC_DIR)/lib $(KLIBC_DIR)/lib/crt0.o
LIBS = -ltcc -ldialog -lcryptal -lbox -lc.obj

all:
	$(KTCC) $(CFLAGS) $(SRC) $(LFLAGS) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
