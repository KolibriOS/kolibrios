KTCC_DIR=../../develop/ktcc/trunk

NAME=whois

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=whois.c
CFLAGS= -I $(KTCC_DIR)/kolibri-libc/include  
LFLAGS= -nostdlib -L $(KTCC_DIR)/kolibri-libc $(KTCC_DIR)/kolibri-libc/crt0.o
LIBS = -lnetwork -lc.obj

all:
	$(KTCC) $(CFLAGS) $(LFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
