KTCC_DIR=../../develop/ktcc/trunk

NAME=whois

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=whois.c
CFLAGS=-I $(KTCC_DIR)/libc/include
LIBS = -lck -lnetwork

all:
	$(KTCC) $(CFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
